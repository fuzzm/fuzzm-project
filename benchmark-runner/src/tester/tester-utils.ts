import { BenchmarkInfo } from './benchmarks';
import { relative, resolve } from 'path';
import {
  AFL_INSTRUMENTER_BIN,
  BENCHMARKS_FOLDER,
  CANARIES_INSTRUMENTER_BIN,
  LAVA_BENCHMARKS_FOLDER,
} from '../constants';
import { exec, ExecException } from 'child_process';

export function getBenchmarkFolder(benchmarkName: string): string {
  if (['base64', 'uniq', 'md5sum'].some((b) => b === benchmarkName)) {
    return resolve(LAVA_BENCHMARKS_FOLDER, benchmarkName);
  }
  return resolve(BENCHMARKS_FOLDER, benchmarkName);
}

export function getDiffTestFolder(benchmarkName: string): string {
  return resolve(getBenchmarkFolder(benchmarkName), 'differential-tests');
}

export async function insertCanaries(
  bin: string,
  binOut: string,
  skipHeap: boolean = false,
  skipStack: boolean = false
): Promise<void> {
  const instrumentCmd = `${CANARIES_INSTRUMENTER_BIN} ${bin} ${binOut} --skip-print ${skipHeap ? '--skip-heap' : ''} ${
    skipStack ? '--skip-stack' : ''
  }`;
  await new Promise<void>((res, rej) => {
    exec(instrumentCmd, (err, stdout, stderr) => {
      if (err) {
        console.log(stdout, stderr);
        rej();
      }
      res();
    });
  });
}

export async function insertAFLInstrumentation(bin: string, binOut: string): Promise<void> {
  const instrumentCmd = `${AFL_INSTRUMENTER_BIN} ${bin} ${binOut}`;
  await new Promise<void>((res, rej) => {
    exec(instrumentCmd, (err, stdout, stderr) => {
      if (err) {
        console.log(stdout, stderr);
        rej();
      }
      res();
    });
  });
}

export async function execute<T>(
  binary: string,
  testFilePath: string,
  outFilePrefix: string,
  benchmarkFolder: string,
  bMarkInfo: BenchmarkInfo,
  cb: (e: ExecException | null, out: string, err: string, timeMS: number) => T
): Promise<T> {
  let cmdTestFilePath = testFilePath;
  if (bMarkInfo.useRelativeInput) {
    cmdTestFilePath = relative(benchmarkFolder, cmdTestFilePath);
  }
  const cmd = bMarkInfo.cmd.replace('@@', cmdTestFilePath).replace('##', outFilePrefix);
  const wasmCmd = `wasmtime ${binary} --dir=${bMarkInfo.useRelativeInput ? '.' : '/'} -- ${cmd}`;
  return new Promise((res) => {
    const hrstart = process.hrtime();
    exec(wasmCmd, { cwd: benchmarkFolder, encoding: 'utf-8' }, (err, stdout, stderr) => {
      const hrend = process.hrtime(hrstart);
      res(cb(err, stdout, stderr, hrend[0] * 1000 + hrend[1] / 1000000));
    });
  });
}
