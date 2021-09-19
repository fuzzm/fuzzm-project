import commander from 'commander';
import { createLogger } from '../logging';
import { basename, resolve } from 'path';
import { Dirent, readdirSync, readFileSync, writeFileSync } from 'fs';
import { Options } from '../options';
import { conf, constructionLatexTable, isDirectory } from '../util';
import { PLOT_FOLDER, RESULTS_DIR } from '../constants';
import { execSync } from 'child_process';
import { dirSync } from 'tmp';
import { Logger } from 'winston';
import { cloneDeep, groupBy, sum } from 'lodash';
import { analyzeCrashes } from './analyze-crashes';
import assert from 'assert';
import { table } from 'table';

interface Repeat {
  // unix time in seconds to a row
  [index: number]: RepeatRow;
}

const log = console.log;

const ignored: string[] = [
  //'abc2mtex',
  //'opj_compress',
  //'pal2rgb',
  //'pdfresurrect',
  //'uniq',
  //'md5sum',
  //'base64',
  //'jbig2dec',
  //'pnm2png',
  //'flac',
];
const SKIP_NATIVE = true;
let logger: Logger;

function getResultsFolders(skipNative: boolean): Dirent[] {
  return readdirSync(RESULTS_DIR, { withFileTypes: true }).filter((f) => {
    const shouldSkipNative = skipNative && f.name.includes('native');
    return f.isDirectory() && !shouldSkipNative;
  });
}

commander
  .arguments('')
  .description('create accumulated statistics for mulitple fuzzer runs of the same benchmark')
  .option('-d, --debug', 'enable debug logging')
  .action(async function (options: any) {
    logger = createLogger('accumulator', options.debug ? 'debug' : 'info');

    // plots
    for (const r of getResultsFolders(SKIP_NATIVE)) {
      if (ignored.some((i) => r.name.includes(i))) continue;
      logger.info(`processing ${r.name}`);
      const [name, platform] = r.name.split('-');
      const rFolder = resolve(RESULTS_DIR, r.name);
      const res = getNewestResults(rFolder);
      benchmarkPlots(resolve(res.tmpDir, basename(res.zipName, '.zip')), name, platform);
    }

    const resultGroups = groupBy(getResultsFolders(false), (d) => {
      const [name, platform] = d.name.split('-');
      return name;
    });

    const crashResults: CrashResults[] = [];
    for (const gName in resultGroups) {
      if (ignored.some((i) => gName.includes(i))) continue;
      const g = resultGroups[gName];
      if (g.length === 0) {
        throw new Error(`Unexpectedly found no platforms for benchmark ${gName}`);
      }

      const wasmResultFolder = g.find((x) => x.name.includes('wasm'));

      let nativeResultFolder;
      if (g.length > 1) {
        nativeResultFolder = g.find((x) => x.name.includes('native'));
      }

      if (wasmResultFolder) {
        crashResults.push(
          await processBenchmarkCrashes(
            gName,
            resolve(RESULTS_DIR, wasmResultFolder.name),
            nativeResultFolder && resolve(RESULTS_DIR, nativeResultFolder.name)
          )
        );
      } else {
        throw new Error(`native or wasm platform not present in results: ${g.join(', ')}`);
      }
    }

    // generate benchmark statistics strResultsTable
    let strResultsTable: string[][] = [];
    strResultsTable.push([
      'benchmark',
      'Fuzzing time',
      'Crashes',
      'Stack canary induced crashes',
      'Heap canary induced crashes',
      '#Paths',
      'Execs/s',
      'Crashes',
      '#Paths',
      'Execs/s',
    ]);
    let totalCrashes = 0;
    let totalStackCanCrashes = 0;
    let totalHeapCanCrashes = 0;
    let totalWasmExecs = 0;
    let totalWasmPaths = 0;
    let totalNativeCrashes = 0;
    let totalNativeExecs = 0;
    let totalNativePaths = 0;
    const addRow = (b: string) => {
      const r = crashResults.find((x) => x.benchmark === b);
      assert(r, `unknown benchmark ${b}`);
      const transformName = (n: string) => {
        // transform from binary name to library name.
        switch (n) {
          case 'opj_compress':
            return 'openjpeg';
          case 'pal2rgb':
            return 'libtiff';
          case 'pnm2png':
            return 'libpng';
          case '2c980fcd46b027cd64d75d974ee48208868304873b6d1b1ad691c743fa3accc5':
            return 'wq';
          case '3318c71ea11c4a759fa406bd5dec2038245d6b47c55c50b5127368d31949c6a3':
            return 'bfi';
          case '5d913289af2f0ac09bca73b620e0bcc563327a94535494b9e0ca9e474cabff4c':
            return 'bf';
          case 'b61f2422b9f7d490add208cfd8a53b7932f12140a5da01270e56c90b3f378996':
            return 'wasi-example';
          case 'cfa2c75ab461c6f7cdc228ba1c98e22b18bf0e7df637d54bb8f32a6abf703915':
            return 'save';
          case '5c8b3eea45224e5929738e4c5c3d1d1a43ec263a60dd102a96161de6d7012ef0':
            return 'rust python';
          case '8d108de7e07ef1ee070ce2e1f057de253839d0be2c30a99f58889e85cf54449a':
            return 'handlebars-cli';
          case '310e582869203573dbbb7a06d8d5e8f9b457261f3b488bf2a37de85d227450ab':
            return 'clang.wasm';
          case 'a7a817db1c6c9fff26f005fd598cc28b15b35124927ec94822d8aa28cf824a7c':
            return 'libxml2.wasm';
          case '69286bc946266e89095151f5e12c77eae3824379b7b297141cc4cc7b5711832a':
            return 'canonicaljson.wasm';
          case '0a0f242b77afe39fa5ff0cd579d7f6378b5869feae7ee792f416c9df19a5caf1':
            return 'zxing_barcode_reader.wasm';
          case '992205b67977f9c7a0d1ea7568d745e055fa3015d1ab70428feed6d8da60d060':
            return 'sqlite.wasm';
          case '7e48bfd68f658ac4916c291b536e6756c173f6ba63311c67c11ffa4b111f0bff':
            return 'viu.wasm';
          case '6f62a6bc5c8f8e3e12a54e2ecbc5674ccfe1c75f91d8e4dd6ebb3fec422a4d6c':
            return 'qjs.wasm';
          case 'f2d833c3da9a7b149d3d4e39eef8a091b8231cd71ece3e57c4ceafb2dd9714cd':
            return 'wasm-interface.wasm';
          case '4165b9f433d19ee69c15ff5e87dac8ef7f04ce62be95c9a569a0a853048052de':
            return 'qjs.wasm';
          case '990040b990434ac4d48565af09f897c84ee02bf22f7e50e2091bb93b17a023db':
            return 'duk1.wasm';
          case '722665633c07c06ea5ff8bab0c78a1bdad0998b06c769cb27efe9834d60bca3c':
            return 'jq.wasm';
          case 'bf750dfa5ccc0f32f59c865d6581acaae2f07feb1919429f2aafc11199206c66':
            return 'duk2.wasm';
          case 'ee42716385c8c8b1d9ba916fcf52ff2a258a4152f4e54af2262425d1832671fe':
            return 'qr2text.wasm';
          case '674a17949619b77f26560d8fd48ac65fb700ae6eed998ff74d62ce444c387d2f':
            return 'hq9_plus_rs.wasm';
          case 'a5a3bb5ef3d679f84b66655696c28415da174242e3bdae99030c1682156c5d04':
            return 'colcrt.wasm';
          case '20cd76fa23f6760d8c4e94c861b93d3e316faaf7a464851a6b8b43db8ebb5cb6':
            return 'rev.wasm';
          case '65bb7368b55ebd366ce339cf58628750b203ea327e22c362f94dfa8bf8b55493':
            return 'basic.wasm';

          default:
            return n;
        }
      };
      totalCrashes += r.wasmCrashes.mean;
      totalStackCanCrashes += r.wasmStackCanaryCrashes.mean;
      totalHeapCanCrashes += r.wasmHeapCanaryCrashes.mean;
      totalNativeCrashes += r.nativeCrashes.mean;
      totalWasmExecs += r.wasmExecsPerSec.mean;
      totalWasmPaths += r.wasmPathsCovered.mean;
      totalNativeExecs += r.nativeExecsPerSec.mean;
      totalNativePaths += r.nativePathsCovered.mean;
      strResultsTable.push([
        transformName(b),
        `${Math.round(r.wasmTimeSec.mean / (60 * 60))}H`,
        `${r.wasmCrashes.mean} +- ${r.wasmCrashes.conf95.toFixed(1)} ${
          r.wasmLavaCrashes.mean > 0 ? ` (LAVA ${r.wasmLavaCrashes.mean})` : ''
        }`,
        `${r.wasmStackCanaryCrashes.mean} +- ${r.wasmStackCanaryCrashes.conf95.toFixed(1)}`,
        `${r.wasmHeapCanaryCrashes.mean} +- ${r.wasmHeapCanaryCrashes.conf95.toFixed(1)}`,
        `${r.wasmPathsCovered.mean} +- ${r.wasmPathsCovered.conf95.toFixed(1)}`,
        `${r.wasmExecsPerSec.mean} +- ${r.wasmExecsPerSec.conf95.toFixed(1)}`,
        `${r.nativeCrashes.mean} +- ${r.nativeCrashes.conf95.toFixed(1)}`,
        `${r.nativePathsCovered.mean} +- ${r.nativePathsCovered.conf95.toFixed(1)}`,
        `${r.nativeExecsPerSec.mean} +- ${r.nativeExecsPerSec.conf95.toFixed(1)}`,
      ]);
    };
    addRow('opj_compress');
    addRow('pal2rgb');
    addRow('pnm2png');
    addRow('pdfresurrect');
    addRow('abc2mtex');
    addRow('flac');
    addRow('jbig2dec');
    addRow('base64');
    addRow('md5sum');
    addRow('uniq');
    //addRow('2c980fcd46b027cd64d75d974ee48208868304873b6d1b1ad691c743fa3accc5');
    addRow('3318c71ea11c4a759fa406bd5dec2038245d6b47c55c50b5127368d31949c6a3');
    addRow('5d913289af2f0ac09bca73b620e0bcc563327a94535494b9e0ca9e474cabff4c');
    addRow('b61f2422b9f7d490add208cfd8a53b7932f12140a5da01270e56c90b3f378996');
    addRow('cfa2c75ab461c6f7cdc228ba1c98e22b18bf0e7df637d54bb8f32a6abf703915');
    addRow('8d108de7e07ef1ee070ce2e1f057de253839d0be2c30a99f58889e85cf54449a');
    addRow('310e582869203573dbbb7a06d8d5e8f9b457261f3b488bf2a37de85d227450ab');
    addRow('a7a817db1c6c9fff26f005fd598cc28b15b35124927ec94822d8aa28cf824a7c');
    addRow('69286bc946266e89095151f5e12c77eae3824379b7b297141cc4cc7b5711832a');
    addRow('0a0f242b77afe39fa5ff0cd579d7f6378b5869feae7ee792f416c9df19a5caf1');
    addRow('992205b67977f9c7a0d1ea7568d745e055fa3015d1ab70428feed6d8da60d060');
    addRow('7e48bfd68f658ac4916c291b536e6756c173f6ba63311c67c11ffa4b111f0bff');
    //addRow('6f62a6bc5c8f8e3e12a54e2ecbc5674ccfe1c75f91d8e4dd6ebb3fec422a4d6c');
    addRow('f2d833c3da9a7b149d3d4e39eef8a091b8231cd71ece3e57c4ceafb2dd9714cd');
    addRow('4165b9f433d19ee69c15ff5e87dac8ef7f04ce62be95c9a569a0a853048052de');
    addRow('990040b990434ac4d48565af09f897c84ee02bf22f7e50e2091bb93b17a023db');
    addRow('722665633c07c06ea5ff8bab0c78a1bdad0998b06c769cb27efe9834d60bca3c');
    addRow('bf750dfa5ccc0f32f59c865d6581acaae2f07feb1919429f2aafc11199206c66');
    addRow('ee42716385c8c8b1d9ba916fcf52ff2a258a4152f4e54af2262425d1832671fe');
    addRow('674a17949619b77f26560d8fd48ac65fb700ae6eed998ff74d62ce444c387d2f');
    addRow('a5a3bb5ef3d679f84b66655696c28415da174242e3bdae99030c1682156c5d04');
    addRow('20cd76fa23f6760d8c4e94c861b93d3e316faaf7a464851a6b8b43db8ebb5cb6');
    //addRow('65bb7368b55ebd366ce339cf58628750b203ea327e22c362f94dfa8bf8b55493');
    strResultsTable.push([
      'Total',
      '',
      String(totalCrashes.toFixed(1)),
      String(totalStackCanCrashes.toFixed(1)),
      String(totalHeapCanCrashes.toFixed(1)),
      '',
      '',
      String(totalNativeCrashes.toFixed(1)),
      '',
      '',
    ]);
    const l = crashResults.length;
    strResultsTable.push([
      'Avg',
      '',
      (totalCrashes / l).toFixed(1),
      (totalStackCanCrashes / l).toFixed(1),
      (totalHeapCanCrashes / l).toFixed(1),
      (totalWasmPaths / l).toFixed(1),
      (totalWasmExecs / l).toFixed(1),
      (totalNativeCrashes / l).toFixed(1),
      (totalNativePaths / l).toFixed(1),
      (totalNativeExecs / l).toFixed(1),
    ]);
    console.log(table(strResultsTable));
    console.log(constructionLatexTable(strResultsTable));
  })
  .parse(process.argv);

type NewestResults = {
  tmpDir: string;
  zipName: string;
};
function getNewestResults(resultsFolder: string): NewestResults {
  const resultZips = readdirSync(resultsFolder, { withFileTypes: true }).filter((f) => f.name.endsWith('.zip'));

  const ds = [];

  const dateRegexp = /(\d+)-(\d+)-(\d+)_(\d+):(\d+)/;
  for (const f of resultZips) {
    const m = f.name.match(dateRegexp);
    if (m) {
      const day = Number(m[1]) > 9 ? m[1] : `0${m[1]}`;
      const month = Number(m[2]) > 9 ? m[2] : `0${m[2]}`;
      const year = m[3];
      const hour = Number(m[4]) > 9 ? m[4] : `0${m[4]}`;
      const min = Number(m[5]) > 9 ? m[5] : `0${m[5]}`;
      const isoDateStr = `${year}-${month}-${day}T${hour}:${min}:00.000Z`;
      const date = Date.parse(isoDateStr);
      ds.push({ f, date });
    }
  }
  logger.debug(`sorting through [${ds.join(', ')}]`);
  // newest first
  ds.sort((a, b) => b.date - a.date);
  const newestZip = ds[0].f.name;

  // unsafeCleanup => deletion if non-empty
  const tmpDir = dirSync({ unsafeCleanup: true });
  const unzipCmd = `unzip ${newestZip} -d ${tmpDir.name}`;
  logger.debug(`running: ${unzipCmd}`);
  execSync(unzipCmd, { cwd: resultsFolder, stdio: 'ignore' });
  return { tmpDir: tmpDir.name, zipName: newestZip };
}

function benchmarkPlots(resultsFolder: string, name: string, platform: string) {
  const optionsFile = resolve(resultsFolder, 'options.json');
  const benchmarkOptions: Options = JSON.parse(readFileSync(optionsFile, { encoding: 'utf-8' }));

  const repeats = getRepeats(resultsFolder);
  verifyRepeats(repeats, name, benchmarkOptions.time);
  normalizeRepeats(repeats);

  plot('uniqueCrashes', repeats, name, platform, benchmarkOptions.time);
  plot('pathsTotal', repeats, name, platform, benchmarkOptions.time);
}

type MetricStat = {
  mean: number;
  conf95: number;
  total: number;
};
type CrashResults = {
  benchmark: string;
  wasmTimeSec: MetricStat;
  wasmCrashes: MetricStat;
  wasmStackCanaryCrashes: MetricStat;
  wasmHeapCanaryCrashes: MetricStat;
  wasmExecsPerSec: MetricStat;
  wasmLavaCrashes: MetricStat;
  wasmPathsCovered: MetricStat;
  nativeTimeSec: MetricStat;
  nativeCrashes: MetricStat;
  nativeExecsPerSec: MetricStat;
  nativePathsCovered: MetricStat;
};

async function processBenchmarkCrashes(
  name: string,
  wasmResultsDir: string,
  nativeResultsDir?: string
): Promise<CrashResults> {
  const newestWasmResults = getNewestResults(wasmResultsDir);
  const newestWasmResultsDir = resolve(newestWasmResults.tmpDir, basename(newestWasmResults.zipName, '.zip'));

  let newestNativeResults;
  let newestNativeResultsDir: string | undefined;
  let nativeRepeatDirs;
  if (nativeResultsDir) {
    newestNativeResults = getNewestResults(nativeResultsDir);
    newestNativeResultsDir = resolve(newestNativeResults.tmpDir, basename(newestNativeResults.zipName, '.zip'));
    nativeRepeatDirs = readdirSync(newestNativeResultsDir).filter((x) =>
      isDirectory(resolve(newestNativeResultsDir as string, x))
    );
  }
  const wasmRepeatDirs = readdirSync(newestWasmResultsDir).filter((x) => isDirectory(resolve(newestWasmResultsDir, x)));


  let wasmCrashes: number[] = [];
  let wasmStackCanaryCrashes: number[] = [];
  let wasmHeapCanaryCrashes: number[] = [];
  let wasmExecsPerSec: number[] = [];
  let wasmLavaCrashes: number[] = [];
  let wasmRunningTime: number[] = [];
  let wasmPathsCovered: number[] = [];
  let nativeCrashes: number[] = [];
  let nativeExecsPerSec: number[] = [];
  let nativeRunningTime: number[] = [];
  let nativePathsCovered: number[] = [];

  for (const rDir of wasmRepeatDirs) {
    let nativeRDir;
    if (nativeRepeatDirs) {
      nativeRDir = nativeRepeatDirs.find((d) => d === rDir);
      if (!nativeRDir) {
        throw new Error(`could not find corresponding native repeat directory for folder ${rDir}`);
      }
    }
    const wasmFuzzerStats = readFileSync(resolve(newestWasmResultsDir, rDir, 'fuzzer_stats'), 'utf-8');
    wasmExecsPerSec.push(getFuzzStat(wasmFuzzerStats, 'execs_per_sec'));
    wasmCrashes.push(getFuzzStat(wasmFuzzerStats, 'unique_crashes'));

    if (newestNativeResultsDir && nativeRDir) {
      const nativeFuzzerStats = readFileSync(resolve(newestNativeResultsDir, nativeRDir, 'fuzzer_stats'), 'utf-8');
      nativeExecsPerSec.push(getFuzzStat(nativeFuzzerStats, 'execs_per_sec'));
      nativeCrashes.push(getFuzzStat(nativeFuzzerStats, 'unique_crashes'));
      nativeRunningTime.push(getRunningTimeFromFuzzerStats(nativeFuzzerStats));
      nativePathsCovered.push(getFuzzStat(nativeFuzzerStats, 'paths_total'));
    } else {
      nativeExecsPerSec.push(NaN);
      nativeCrashes.push(NaN);
      nativeRunningTime.push(NaN);
      nativePathsCovered.push(NaN);
    }

    const rFolder = resolve(newestWasmResultsDir, rDir, 'crashes');
    const wasmRes = await analyzeCrashes(name, rFolder);
    wasmStackCanaryCrashes.push(wasmRes.stackCanaryCrashes);
    wasmHeapCanaryCrashes.push(wasmRes.heapCanaryCrashes);
    wasmLavaCrashes.push(wasmRes.lavaCrashes);
    wasmRunningTime.push(getRunningTimeFromFuzzerStats(wasmFuzzerStats));
    wasmPathsCovered.push(getFuzzStat(wasmFuzzerStats, 'paths_total'));
  }

  const s = (xs: number[]): MetricStat => {
    const cData = conf(xs);
    return {
      mean: cData.mean,
      conf95: cData.interval,
      total: sum(xs),
    };
  };

  return {
    benchmark: name,
    nativeCrashes: s(nativeCrashes),
    nativeExecsPerSec: s(nativeExecsPerSec),
    nativeTimeSec: s(nativeRunningTime),
    nativePathsCovered: s(nativePathsCovered),
    wasmStackCanaryCrashes: s(wasmStackCanaryCrashes),
    wasmCrashes: s(wasmCrashes),
    wasmExecsPerSec: s(wasmExecsPerSec),
    wasmHeapCanaryCrashes: s(wasmHeapCanaryCrashes),
    wasmLavaCrashes: s(wasmLavaCrashes),
    wasmTimeSec: s(wasmRunningTime),
    wasmPathsCovered: s(wasmPathsCovered),
  };
}

function plot(metric: RowProperty, repeats: Repeat[], name: string, platform: string, timeMinutes: number) {
  logger.debug(`creating plot for property ${metric}`);
  const plot = meanWithConfidencePlot(repeats, metric);

  const outputFileName = `${name}-${platform}-${metric}.dat`;
  const dataFile = resolve(PLOT_FOLDER, outputFileName);
  writeFileSync(dataFile, plot);

  // create the plot
  const plotFile = resolve(PLOT_FOLDER, `${name}-${platform}-${metric}.png`);
  //const title = `${name} ${repeats.length}*${timeMinutes / 60}H`;
  const title = name.replace('_', '\\_');
  const createPlotCmd = `gnuplot -e "datafile='${dataFile}'"\
   -e "outfile='${plotFile}'"\
   -e "metric='${metric == 'pathsTotal' ? 'Paths' : metric === 'uniqueCrashes' ? 'Crashes' : metric}'"\
   -e "plottitle='${title}'" plot-with-conf.plg`;
  execSync(createPlotCmd, { cwd: PLOT_FOLDER, stdio: 'ignore' });
}

type RowProperty =
  | 'cyclesDone'
  | 'curPath'
  | 'pathsTotal'
  | 'pendingTotal'
  | 'pendingFavs'
  | 'mapSize'
  | 'uniqueCrashes'
  | 'uniqueHangs'
  | 'maxDepth'
  | 'execsPerSec';
type RepeatRow = Record<RowProperty, number>;

/**
 * returns the running time in seconds
 */
function getRunningTimeFromFuzzerStats(fuzzerStats: string): number {
  const sTime = getFuzzStat(fuzzerStats, 'start_time');
  const eTime = getFuzzStat(fuzzerStats, 'last_update');
  return eTime - sTime;
}

function getFuzzStat(fuzzerStats: string, stat: string): number {
  // fixme only works with natural numbered stats
  const rStat = new RegExp(`${stat}\\s+:\\s(\\d+)`);
  const res = rStat.exec(fuzzerStats);
  if (res) {
    return Number(res[1]);
  }
  throw new Error(`unable to extract ${stat} from\n${fuzzerStats}`);
}

function getRepeats(resultsFolder: string): Repeat[] {
  let runs: Repeat[] = [];

  const repeatDirs = readdirSync(resultsFolder).filter((x) => isDirectory(resolve(resultsFolder, x)));
  for (const rDir of repeatDirs) {
    const repeat: Repeat = {};
    const rFolder = resolve(resultsFolder, rDir);

    // fetch fuzzer stats
    const fuzzerStatsFile = resolve(rFolder, 'fuzzer_stats');
    const fuzzerStats = readFileSync(fuzzerStatsFile, { encoding: 'utf-8' });
    const elapsedTimeSec = getRunningTimeFromFuzzerStats(fuzzerStats);

    // fetch plot data
    const plotDataFile = resolve(rFolder, 'plot_data');
    const plotData = readFileSync(plotDataFile, { encoding: 'utf-8' });
    const lines = plotData.split('\n');
    const initTime = Number(lines[1].slice(0, lines[1].indexOf(',')));
    let lastUpdateTime: number = 0;
    lines
      // first line is a comment
      .slice(1)
      .map((strRow) => {
        // last line is empty.
        if (strRow !== '') {
          const data = strRow.replace(' ', '').split(',');
          lastUpdateTime = Number(data[0]) - initTime;
          repeat[lastUpdateTime] = {
            cyclesDone: Number(data[1]),
            curPath: Number(data[2]),
            pathsTotal: Number(data[3]),
            pendingTotal: Number(data[4]),
            pendingFavs: Number(data[5]),
            mapSize: Number(data[6].replace('%', '')),
            uniqueCrashes: Number(data[7]),
            uniqueHangs: Number(data[8]),
            maxDepth: Number(data[9]),
            execsPerSec: Number(data[10]),
          };
        }
      });

    if (lastUpdateTime < elapsedTimeSec) {
      // AFL only updates the plot file after interesting changes and it's therefore likely
      // that the latest update in the plot file happened much earlier than the fuzzer terminated.
      // To avoid mixing this case with cases where the fuzzer terminates early, we insert an
      // artificial entry in the runs at the time where the fuzzer execution ended.
      // This entry will hold the same values as the last update in the plot file.
      const d = repeat[lastUpdateTime];
      repeat[elapsedTimeSec] = cloneDeep(d);
    }

    runs.push(repeat);
  }

  return runs;
}

/**
 * returns the union of all time values in the repeats
 * @param repeats
 */
function getTimeValues(repeats: Repeat[]): number[] {
  let timeValues: number[] = [];
  repeats.forEach((r) => {
    for (let time in r) {
      const timeNum = Number(time);
      if (!timeValues.includes(timeNum)) {
        timeValues.push(timeNum);
      }
    }
  });
  return timeValues;
}

/**
 * The repeats may not all have the same X values, i.e., recordings for the same time.
 * This function adds the missing entries by a weighted average of the nearest neighbours.
 *
 * Assume x1 < x2 < x3 and repeat_i lacks an y value (y2) for x2.
 * We then let x_interval be x3 - x2 and set y2 = y1 * (x2 - x1)/x_interval + y3 * (x3 - x2)/x_interval.
 *
 * If repeat_i lacks xn and has no entry xn' > xn
 * then let xn1 and xn2 be resp. the largest and second largest x values for repeat_i
 * We let yn = yn1 + (xn2 - xn1)/(xn - xn1) * (yn1 - yn2)
 * I.e., we choose the same slope as between xn2 and xn1 and multiply by the
 * ratio of the distance between xn2 and xn1 in relation to xn1 and xn
 *
 *
 * @param repeats
 */
function normalizeRepeats(repeats: Repeat[]) {
  logger.debug('normalizing repeats');
  let timeValues = getTimeValues(repeats);
  logger.debug(`processing ${repeats.length} repeats and ${timeValues.length} time values`);

  for (const r of repeats) {
    for (const time of timeValues) {
      if (!(time in r)) {
        let timeLess: number = 0,
          // timeLessP is the greatest neighbour of timeLess;
          timeLessP: number = 0,
          timeMore: number = Number.MAX_SAFE_INTEGER;

        // find closets neighbours
        for (const timeR in r) {
          const timeRNum = Number(timeR);
          if (timeRNum > timeLess && timeRNum < time) {
            timeLessP = timeLess;
            timeLess = timeRNum;
          }
          if (timeRNum < timeMore && timeRNum > time) {
            timeMore = timeRNum;
          }
        }

        // 0 is always in r, so there will always be a value for timeLess
        let f: (x: RowProperty) => number;
        if (timeMore === Number.MAX_SAFE_INTEGER) {
          // time is a new max value for this repeat
          f = (p: RowProperty): number =>
            r[timeLess][p] + ((time - timeLess) / (timeLess - timeLessP)) * (r[timeLess][p] - r[timeLessP][p]);
        } else {
          // time is some value where time' < time < time'' and time' and time'' are in this repeat.
          const interval = timeMore - timeLess;
          f = (p: RowProperty): number => {
            return r[timeLess][p] * ((time - timeLess) / interval) + r[timeMore][p] * ((timeMore - time) / interval);
          };
        }
        r[time] = {
          cyclesDone: f('cyclesDone'),
          curPath: f('curPath'),
          pathsTotal: f('pathsTotal'),
          pendingTotal: f('pendingTotal'),
          pendingFavs: f('pendingFavs'),
          mapSize: f('mapSize'),
          uniqueCrashes: f('uniqueCrashes'),
          uniqueHangs: f('uniqueHangs'),
          maxDepth: f('maxDepth'),
          execsPerSec: f('execsPerSec'),
        };
      }
    }
  }
}

/**
 * prints error if some of the repeats looks to have terminated prematurely
 * @param r
 * @param benchmark
 * @param runningTimeMinutes running time in minutes
 */
function verifyRepeats(r: Repeat[], benchmark: string, runningTimeMinutes: number): void {
  for (let i = 0; i < r.length; i++) {
    const keys = Object.keys(r[i]).map((x) => Number(x));
    keys.sort((x, y) => (x < y ? x : y));
    const end = keys[keys.length - 1];
    const maxMinutes = end / 60;

    const absDiff = Math.abs(runningTimeMinutes - maxMinutes);
    if (absDiff > 15) {
      console.error(`A run for ${benchmark} diverge with ${absDiff.toFixed(2)} minutes from the runningTime option`);
    }
  }
}

/**
 * returns columns:
 * #time #mean #95%bot #95%top
 */
function meanWithConfidencePlot(repeats: Repeat[], p: RowProperty): string {
  let res = `#time #mean #95%top #95%bot\n`;

  const times = getTimeValues(repeats);

  for (const t of times) {
    let yValues = [];
    for (const r of repeats) {
      yValues.push(r[t][p]);
    }

    const s = (x: number) => x.toFixed(2);
    const c = conf(yValues);
    res += `${t} ${s(c.mean)} ${s(c.mean - c.interval)} ${s(c.mean + c.interval)}\n`;
  }

  return res;
}
