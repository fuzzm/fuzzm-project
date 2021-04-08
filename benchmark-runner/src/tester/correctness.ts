import { readFileSync, readdirSync, unlinkSync, writeFileSync } from 'fs';
import { resolve, basename } from 'path';
import { dirSync } from 'tmp';
import chalk from 'chalk';
import { BenchmarkInfo, benchmarks } from './benchmarks';
import {
  execute,
  getBenchmarkFolder,
  getDiffTestFolder,
  insertAFLInstrumentation,
  insertCanaries,
} from './tester-utils';

(async function () {
  for (const b in benchmarks) {
    console.log(b);
    const bMarkInfo = (benchmarks as any)[b] as BenchmarkInfo;
    const benchmarkFolder = getBenchmarkFolder(b);
    const testFolder = getDiffTestFolder(b);
    const testFiles = readdirSync(testFolder);
    // unsafeCleanup => deletion if non-empty
    const outputFolder = dirSync({ unsafeCleanup: true });

    for (const testFile of testFiles) {
      let testFilePath = resolve(testFolder, testFile);
      const wasmBin = resolve(benchmarkFolder, `${bMarkInfo.binName}`);
      const outFilePrefix = resolve(outputFolder.name, testFile);

      async function execBench(bin: string): Promise<string> {
        return await execute(bin, testFilePath, outFilePrefix, benchmarkFolder, bMarkInfo, (e, stdout, stderr) => {
          if (bMarkInfo.outputFile === 'stdout') {
            return `${stdout}${stderr}`;
          } else {
            const outputFile = bMarkInfo.outputFile.replace('##', outFilePrefix);
            const content = readFileSync(outputFile, { encoding: 'utf-8' });
            unlinkSync(outputFile);
            return content;
          }
        });
      }

      const wasmOutput = await execBench(wasmBin);

      const instrumentedBin = `${wasmBin}.instr`;
      await insertAFLInstrumentation(wasmBin, instrumentedBin);
      await insertCanaries(instrumentedBin, instrumentedBin);
      const instrWasmOut = await execBench(instrumentedBin);

      if (wasmOutput == instrWasmOut) {
        console.log(chalk.green(`\t${testFile} passed`));
      } else {
        console.log(chalk.red(`\t${testFile} did not pass wasm:`));
        writeFileSync(resolve(outputFolder.name, 'wasm.out'), wasmOutput);
        writeFileSync(resolve(outputFolder.name, 'wasm.instr.out'), instrWasmOut);
        console.log(`failed tests output in ${outputFolder.name}`);
        process.exit(1);
      }
    }

    outputFolder.removeCallback();
  }
})();
