#!/usr/bin/env node
import { ExecException } from 'child_process';
import { AFL_FOLDER } from '../constants';

const fs = require('fs');
const cp = require('child_process');
const p = require('path');
import { fileSync } from 'tmp';
import { copyFileSync } from 'fs';

export type AnalyzedCrashesResults = {
  crashes: number;
  stackCanaryCrashes: number;
  heapCanaryCrashes: number;
  inconclusiveCrashes: number;
  lavaCrashes: number;
};

export async function analyzeCrashes(benchmark: string, crashFolder: string): Promise<AnalyzedCrashesResults> {
  let preFileArg = '';
  let args: string[] = [];
  let nativeBinary: string;
  let relativePath = false;
  let folder: string | undefined = undefined;

  switch (benchmark) {
    case 'pal2rgb':
      args = ['/dev/null'];
      folder = 'programs/pal2rgb-wasm';
      nativeBinary = 'programs/pal2rgb/prog';
      break;
    case 'opj_compress':
      preFileArg = '-i';
      args = ['-o', 'programs/opj_compress-wasm/out.jp2'];
      folder = 'programs/opj_compress-wasm';
      nativeBinary = 'programs/opj_compress/prog';
      break;
    case 'pdfresurrect':
      args = [''];
      folder = 'programs/pdfresurrect-wasm';
      nativeBinary = 'programs/pdfresurrect/prog';
      break;
    case 'pnm2png':
      args = ['/dev/null'];
      folder = 'programs/pnm2png-wasm';
      nativeBinary = 'programs/pnm2png/prog';
      break;
    case 'palbart':
      args = [''];
      folder = 'programs/palbart-wasm';
      nativeBinary = 'programs/palbart/prog';
      // Otherwise it complains about too long path names.
      relativePath = true;
      break;
    case 'base64':
      preFileArg = '--decode';
      folder = 'programs/base64-wasm';
      nativeBinary = 'programs/base64/prog';
      break;
    case 'md5sum':
      preFileArg = '-c';
      folder = 'programs/md5sum-wasm';
      nativeBinary = 'programs/md5sum/prog';
      break;
    case 'uniq':
      folder = 'programs/uniq-wasm';
      nativeBinary = 'programs/uniq/prog';
      break;
    case 'abc2mtex':
      folder = 'programs/abc2mtex-wasm';
      nativeBinary = 'programs/abc2mtex/prog';
      break;
    case 'jbig2dec':
      folder = 'programs/jbig2dec-wasm';
      nativeBinary = 'programs/abc2mtex/prog';
      args = ['-t', 'png', '-o', '/dev/null'];
      break;
    case 'flac':
      folder = 'programs/flac-wasm';
      nativeBinary = 'programs/flac/prog';
      args = ['-f', '-d'];
      break;
  }

  if (!folder) {
    throw new Error(`unexpected benchmark ${benchmark}`);
  }

  const wasmBinary = p.resolve(AFL_FOLDER, folder, 'prog.wasm');
  const instrWasmBinary = p.resolve(AFL_FOLDER, folder, 'prog.wasm.instr');

  const results: any = {};
  async function run(execStr: string): Promise<[string, string, number, boolean]> {
    return new Promise((res) => {
      cp.exec(
        execStr,
        { stdio: 'ignore', timeout: 5000 },
        (err: ExecException | null, stdout: string | Buffer, stderr: string | Buffer) => {
          const timedout = err && err.signal === 'SIGTERM';
          res([stderr.toString(), stdout.toString(), (err && err.code) || 0, !!timedout]);
        }
      );
    });
  }

  async function runWasmBin(bin: string, crashFile: string): Promise<[string, string, number, boolean]> {
    const execStr = `wasmtime ${bin} --dir=. --dir=/ -- ${preFileArg || ''} ${crashFile} ${args.join(' ')}`;
    return run(execStr);
  }

  async function runNativeBin(bin: string, crashFile: string): Promise<[string, string, number, boolean]> {
    const execStr = `${bin} ${preFileArg || ''} ${crashFile} ${args.join(' ')}`;
    return run(execStr);
  }

  /*
   * {reason, error}
   */
  function getReason(
    stderr: string,
    stdout: string,
    code_: number,
    timedout: boolean
  ): { reason: string; error: string } {
    if (timedout) return { reason: 'timedout', error: stderr };

    const reasonRegexp = /wasm trap: (.+)/;
    const m = reasonRegexp.exec(stderr);
    let reason = m ? m[1] : 'no trap';
    if (reason === 'unreachable') {
      // check if the top element in the stack trace is 'dlfree' or 'dlmalloc'
      const isHeapCanaryCrashRegexp = /0(.+)(dlfree|dlrealloc)/;
      const isHeapCanaryCrash = stderr.match(isHeapCanaryCrashRegexp);
      if (isHeapCanaryCrash) {
        reason = 'heap canary';
      } else {
        reason = 'stack canary';
      }
    }

    const lava_regexp = /Successfully triggered bug (\d+), crashing now!/;
    const l = lava_regexp.exec(stdout);
    reason = l ? `LAVA BUG ${l[1]}` : reason;

    return { reason, error: stderr };
  }

  const crashFiles = fs.readdirSync(crashFolder).filter((f: string) => !f.includes('README'));
  const count = crashFiles.length;
  let diffCount = 0;

  let stackCanaryCrashes = 0;
  let heapCanaryCrashes = 0;

  // crash triggered by a bug injected in LAVA-M
  let lavaCrashes = 0;
  while (crashFiles.length) {
    await Promise.all(
      crashFiles.splice(0, 1).map(async (crashFile: string) => {
        type reason = { reason: string; error: string };
        const res: { wasm?: reason; wasmInstr?: reason; native?: number } = {};

        let crashFilePath = p.join(crashFolder, crashFile);

        // move crash file to a tmp file since some benchmarks don't like symbols like ':' in the path/name of crash files.
        // use .bmp postfix for the opj_compress benchmark otherwise it complains about the file type.
        const crashFileTmpObj = benchmark === 'opj_compress' ? fileSync({ postfix: '.bmp' }) : fileSync();
        let crashFileTmp = crashFileTmpObj.name;
        copyFileSync(crashFilePath, crashFileTmp);
        if (relativePath) {
          crashFileTmp = p.relative(__dirname, crashFileTmp);
        }

        res.wasm = getReason(...(await runWasmBin(wasmBinary, crashFileTmp)));
        res.wasmInstr = getReason(...(await runWasmBin(instrWasmBinary, crashFileTmp)));
        res.native = (await runNativeBin(nativeBinary, crashFileTmp))[2];
        results[crashFile] = res;

        const diff = res.wasm.reason !== res.wasmInstr.reason;
        if (diff) diffCount++;

        if (diff && res.wasmInstr.reason.includes('heap canary')) stackCanaryCrashes++;
        if (diff && res.wasmInstr.reason.includes('stack canary')) heapCanaryCrashes++;

        if (res.wasmInstr.reason.includes('LAVA')) lavaCrashes++;

        crashFileTmpObj.removeCallback();
      })
    );
  }

  return {
    stackCanaryCrashes: stackCanaryCrashes,
    heapCanaryCrashes: heapCanaryCrashes,
    crashes: count,
    inconclusiveCrashes: diffCount - stackCanaryCrashes,
    lavaCrashes,
  };
}
