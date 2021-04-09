import { BenchmarkInfo, benchmarks } from './benchmarks';
import {
  execute,
  getBenchmarkFolder,
  getDiffTestFolder,
  insertAFLInstrumentation,
  insertCanaries,
} from './tester-utils';
import { readdirSync } from 'fs';
import { dirSync } from 'tmp';
import { resolve } from 'path';
import { table } from 'table';
import { conf, constructionLatexTable } from '../util';

const NUM_RUNS = 25;

(async function () {
  let resultsTable: (string | number | Measure | undefined)[][] = [
    ['Benchmark', '#testFiles', 'Raw', 'Stack Can.', 'Heap Can.', 'All Can.', 'Coverage', 'ALL'],
  ];
  type Measure = {
    mean: number;
    // The interval size, i.e., 95% conf is [mean-conf95, mean + conf95]
    conf95: number;
  };
  type MT = Measure | undefined;
  let totalRow: [string, number, number, MT, MT, MT, MT, MT] = [
    'total',
    0,
    0,
    undefined,
    undefined,
    undefined,
    undefined,
    undefined,
  ];
  for (const b in benchmarks) {
    console.log(`processing ${b}`);
    const bMarkInfo = (benchmarks as any)[b] as BenchmarkInfo;
    const benchmarkFolder = getBenchmarkFolder(b);
    const testFolder = getDiffTestFolder(b);
    const testFiles = readdirSync(testFolder);
    // unsafeCleanup => deletion if non-empty
    const outputFolder = dirSync({ unsafeCleanup: true });

    async function measurePerformance(bin: string): Promise<number[]> {
      let measures: number[] = [];
      for (let i = 0; i < NUM_RUNS; i++) {
        let totalTestTime = 0;
        for (const t of testFiles) {
          const outFilePrefix = resolve(outputFolder.name, t);
          let testFilePath = resolve(testFolder, t);
          const timeMS = await execute<number>(
            bin,
            testFilePath,
            outFilePrefix,
            benchmarkFolder,
            bMarkInfo,
            (err, stdout, stderr, execTime) => {
              return execTime;
            }
          );
          totalTestTime += timeMS;
        }
        measures.push(totalTestTime);
      }

      return measures;
    }
    const rawBin = resolve(benchmarkFolder, `${bMarkInfo.binName}`);
    const totalTimeRaw = await measurePerformance(rawBin);

    // only stack
    const stackBin = `${rawBin}.stack`;
    await insertCanaries(rawBin, stackBin, true, false);
    const totalTimeStack = await measurePerformance(stackBin);

    // only heap
    const heapBin = `${rawBin}.heap`;
    await insertCanaries(rawBin, heapBin, false, true);
    const totalTimeHeap = await measurePerformance(heapBin);

    // all canaries
    const canariesBin = `${rawBin}.can`;
    await insertCanaries(rawBin, canariesBin, false, false);
    const totalTimeCan = await measurePerformance(canariesBin);

    // afl
    const aflBin = `${rawBin}.afl`;
    await insertAFLInstrumentation(rawBin, aflBin);
    const totalTimeAFL = await measurePerformance(aflBin);

    // all
    const allBin = `${rawBin}.all`;
    await insertCanaries(aflBin, allBin, false, false);
    const totalTimeAll = await measurePerformance(allBin);

    const meanRawTime = conf(totalTimeRaw).mean;
    const ratio = (t: number[]) => {
      const ratios = t.map((t) => t / meanRawTime);
      const cData = conf(ratios);
      return {
        mean: cData.mean,
        conf95: cData.interval,
      };
    };
    resultsTable.push([
      b,
      String(testFiles.length),
      `${meanRawTime.toFixed(2)}ms`,
      ratio(totalTimeStack),
      ratio(totalTimeHeap),
      ratio(totalTimeCan),
      ratio(totalTimeAFL),
      ratio(totalTimeAll),
    ]);

    const plusMeasure = (m1: Measure | undefined, m2: Measure): Measure => {
      const m1Mean = m1 ? m1.mean : 0;
      const m1Conf95 = m1 ? m1.conf95 : 0;
      return {
        mean: m1Mean + m2.mean,
        conf95: m1Conf95 + m2.conf95,
      };
    };

    totalRow[1] += testFiles.length;
    totalRow[2] += meanRawTime;
    totalRow[3] = plusMeasure(totalRow[3], ratio(totalTimeStack));
    totalRow[4] = plusMeasure(totalRow[4], ratio(totalTimeHeap));
    totalRow[5] = plusMeasure(totalRow[5], ratio(totalTimeCan));
    totalRow[6] = plusMeasure(totalRow[6], ratio(totalTimeAFL));
    totalRow[7] = plusMeasure(totalRow[7], ratio(totalTimeAll));
  }

  const avg = (x: number | Measure) => {
    const b = Object.keys(benchmarks).length;
    if (typeof x === 'number') {
      return (x / b).toFixed(2);
    } else {
      return {
        mean: x.mean / b,
        conf95: x.conf95 / b,
      };
    }
  };
  const avgRow = [
    'average',
    avg(totalRow[1]),
    avg(totalRow[2]),
    avg(totalRow[3] as Measure),
    avg(totalRow[4] as Measure),
    avg(totalRow[5] as Measure),
    avg(totalRow[6] as Measure),
    avg(totalRow[7] as Measure),
  ];
  resultsTable.push(avgRow);
  resultsTable.push(totalRow);
  const strResultsTable = resultsTable.map((r) => {
    return r.map((x) => {
      if (typeof x === 'number') {
        return String(x.toFixed(2));
      } else if (typeof x === 'object') {
        return `${x.mean.toFixed(2)} +- ${x.conf95.toFixed(2)}`;
      } else if (typeof x === 'undefined') {
        return '';
      } else {
        return x;
      }
    });
  });
  console.log(table(strResultsTable));
  console.log(constructionLatexTable(strResultsTable));
})();
