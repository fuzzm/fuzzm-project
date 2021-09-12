import commander from 'commander';
import { Option } from 'commander';
import { AFL_FOLDER, FUZZ_COMMANDS_NATIVE, FUZZ_COMMANDS_WASM, RESULTS_DIR } from './constants';
import { createLogger } from './logging';
import { ChildProcessByStdio, exec, spawn } from 'child_process';
import { cloneDeep } from 'lodash';
import { resolve, basename, join } from 'path';
import { createDirectoryIfMissing, isEmptyDirectory } from './util';
import { copy, remove, writeFile } from 'fs-extra';
import { existsSync, statSync } from 'fs';
import { Options } from './options';
import { Readable } from 'stream';
const process = require('process');

console.log(AFL_FOLDER);

/**
 * Run a benchmark with configurations:
 *  - number of hours
 *  - number of runs
 *  - parallel? (probably not)
 *  - wasm_mode?
 *
 *  copy results to some destination.
 *  create graphs, means etc. from the results?
 */

let fuzzProcess: ChildProcessByStdio<null, Readable, Readable>;
let deadlockInterval: NodeJS.Timeout;
let cancellationTimeout: NodeJS.Timeout;
let currentRunStartTime: Date;

commander
  .arguments('<benchmark-name>')
  .description('runs a fuzzing benchmark')
  .option('-d, --debug', 'enable debug logging')
  .addOption(new Option('-r, --repeat <repeats>', 'number of times to repeat the fuzz').default(1, 'run once'))
  .addOption(new Option('-t, --time <minutes>', 'number of times to repeat the fuzz').default(60, '1 hour'))
  .addOption(new Option('-n, --native', 'run in native mode').default(false, 'wasm mode'))
  .action(async function (name: string, options: Options) {
    // options are strings if the default value isn't used.
    options.repeat = Number(options.repeat);
    options.time = Number(options.time);

    const withPlatformName = `${name}${!options.native ? '-wasm' : ''}`;
    const logger = createLogger(withPlatformName, options.debug ? 'debug' : 'info');
    logger.info(`running fuzzing benchmark for ${name} with options: ${JSON.stringify(options, null, 2)}`);

    // assert that no existing results are in the fuzzing directory
    const benchmarkDirectory = resolve(AFL_FOLDER, 'programs', withPlatformName);
    const findingsDirectory = resolve(benchmarkDirectory, 'findings');
    if (!(await isEmptyDirectory(findingsDirectory))) {
      logger.error(`findings directory for benchmark ${name} is non-empty`);
      process.exit(1);
    }

    // find fuzzing command
    const fuzzCmd = options.native ? FUZZ_COMMANDS_NATIVE[name] : FUZZ_COMMANDS_WASM[name];
    if (!fuzzCmd) {
      logger.error(`unknown benchmark ${name}`);
      process.exit(1);
    }

    // create results directory
    const d = new Date();
    const timestamp = `${d.getDate()}-${d.getMonth()}-${d.getFullYear()}_${d.getHours()}:${d.getMinutes()}`;
    const benchmarkResultDir = resolve(RESULTS_DIR, `${name}${options.native ? '-native' : '-wasm'}`);
    const instanceResultDir = resolve(benchmarkResultDir, timestamp);
    await createDirectoryIfMissing(instanceResultDir);

    // create environment
    const env = cloneDeep(process.env);
    if (!options.native) {
      (env as any).WASM_MODE = 1;
    }
    (env as any).AFL_SKIP_CPUFREQ = 1;

    if (fuzzCmd.unstableWasiAPI) {
      (env as any).WASI_VERSION = "wasi_unstable";
    }

    for (let i = 0; i < options.repeat; i++) {
      let success = false;
      while (!success) {
        try {
          await new Promise<void>((res, rej) => {
            let fuzzArguments: string[];
            const progFolder = join('programs', fuzzCmd.name);
            const inputFolder = join(progFolder, 'test-case');
            const outputFolder = join(progFolder, 'findings');
            const wasm = fuzzCmd.name.includes('wasm');
            const prog = join(progFolder, wasm ? 'prog.wasm.instr' : 'prog');
            fuzzArguments = ['-m8000M', '-i', inputFolder, '-o', outputFolder];

            if (fuzzCmd.inputFile) {
              fuzzArguments = fuzzArguments.concat(['-f', fuzzCmd.inputFile]);
            }

            if (fuzzCmd.dictionary) {
              const dictionaryAbs = resolve(AFL_FOLDER, fuzzCmd.dictionary);
              if (existsSync(dictionaryAbs)) {
                fuzzArguments = fuzzArguments.concat(['-x', fuzzCmd.dictionary]);

                // set dictionary field such that it is written to the options.json file
                (options as any).dictionary = fuzzCmd.dictionary;
              } else {
                logger.info(`missing dictionary for ${fuzzCmd.name}. Will run without`);
              }
            }

            fuzzArguments = fuzzArguments.concat(['--', prog, ...fuzzCmd.cmd.split(' ')]);
            const cmd = './afl-fuzz';

            logger.info(`fuzzing ${name}: ${options.time}m - repeat ${i}`);

            const envString = `AFL_SKIP_CPUFREQ=${env['AFL_SKIP_CPUFREQ'] === 1 ? '1' : '0'} WASM_MODE=${
              env['WASM_MODE'] === 1 ? '1' : '0'
            }`;

            currentRunStartTime = new Date();
            logger.debug(`running ${envString} ${cmd} ${fuzzArguments.join(' ')}`);
            fuzzProcess = spawn(cmd, fuzzArguments, {
              cwd: AFL_FOLDER,
              //stdio: ['inherit', 'inherit', 'inherit'],
              stdio: ['inherit', 'pipe', 'pipe'],
              env,
              detached: true,
            });
            logger.debug(`started fuzzing process ${fuzzProcess.pid}`);

            let modifiedLast: number | undefined = undefined;
            /**
             * deadlock interval will continuously monitor the .cur_input file (or custom file if set)
             * for changes. If no changes occur over some set amount of time, the fuzzing is canceled and restarted
             * This is to mitigate a deadlock that seems to occur in the fuzzer with low probability.
             */
            deadlockInterval = setInterval(() => {
              const inputFile = resolve(AFL_FOLDER, fuzzCmd.inputFile || join(outputFolder, '.cur_input'));
              try {
                const stats = statSync(inputFile);
                let modifiedTime = stats.mtimeMs;

                if (modifiedLast && modifiedTime === modifiedLast) {
                  // 10 seconds without changes indicates a dead lock
                  logger.info(`fuzzing process looks dead. restarting.`);
                  rej();
                  process.kill(-fuzzProcess.pid, 'SIGKILL');
                } else {
                  modifiedLast = modifiedTime;
                }
              } catch (e) {
                // ignore failing stat
                logger.error(`deadlock interval failed with error ${e}`);
              }
            }, 50 * 1000);

            fuzzProcess.on('close', () => {
              logger.debug('close');
              res();
            });

            fuzzProcess.stderr.on('data', (data) => {
              logger.error(data);
            });

            fuzzProcess.stdout.on('data', (data) => {
              logger.info(data);
            });

            cancellationTimeout = setTimeout(() => {
              logger.debug(`stopping fuzzing after ${options.time} minutes`);
              // also kill child processes
              process.kill(-fuzzProcess.pid);
            }, options.time * 1000 * 60);
          });

          function diffMinutes(dt2: Date, dt1: Date) {
            let diff = (dt2.getTime() - dt1.getTime()) / 1000;
            diff /= 60;
            return Math.abs(Math.round(diff));
          }

          const now = new Date();
          const elapsedTime = diffMinutes(now, currentRunStartTime);
          if (options.time - elapsedTime > 30) {
            logger.error(
              `restarting fuzzing since ${elapsedTime} is more than 60 minutes less than specified time ${options.time}`
            );
            await remove(findingsDirectory);
            await createDirectoryIfMissing(findingsDirectory);
          } else {
            success = true;
          }
        } catch (e) {
          // fuzzing did not terminate successfully (probably due to a deadlock).
          // remove the findings and try a restart
          await remove(findingsDirectory);
          await createDirectoryIfMissing(findingsDirectory);
        }
        clearInterval(deadlockInterval);
        clearTimeout(cancellationTimeout);
      }

      // copy findings
      const destinationDir = resolve(instanceResultDir, `repeat-${i}`);
      logger.info(`copying results to ${destinationDir}`);
      await copy(findingsDirectory, destinationDir, { overwrite: false });
      // empty findings directory for next run
      await remove(findingsDirectory);
      await createDirectoryIfMissing(findingsDirectory);

      // create plots page using afl-plot
      await new Promise((res) => {
        const aflPlotCmd = `./afl-plot ${destinationDir} ${resolve(destinationDir, 'plot')}`;
        exec(aflPlotCmd, { cwd: AFL_FOLDER, timeout: 1000 * 600 }, (err, stdout, stderr) => {
          if (err) {
            logger.error(`problem creating plots for data in ${destinationDir} ${stderr} ${stdout}`);
          }
          res(true);
        });
      });
    }

    // store options object as json file in the results directory
    await writeFile(resolve(instanceResultDir, 'options.json'), JSON.stringify(options, null, 2));

    //zip the results (to avoid storing all the small files in the git repo)
    await new Promise((res) => {
      // use basename to avoid having all folders from the root in the zip file
      const zipCmd = `zip -r ${basename(instanceResultDir)}.zip ${basename(instanceResultDir)}`;
      exec(zipCmd, { cwd: benchmarkResultDir, timeout: 1000 * 600 }, (err, stdout, stderr) => {
        if (err) {
          logger.error(`problem zipping results from ${name} ${stderr} ${stdout}`);
        }
        res(true);
      });
    });
  })
  .parse(process.argv);

function killAFLOnDie() {
  if (fuzzProcess) {
    process.kill(-fuzzProcess.pid);
  }
  if (deadlockInterval) {
    clearInterval(deadlockInterval);
  }
}

//process.on('SIGKILL', killAFLOnDie);
process.on('SIGINT', killAFLOnDie);
