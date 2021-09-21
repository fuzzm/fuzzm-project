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
  let nativeBinary: string | undefined;
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
    case '2c980fcd46b027cd64d75d974ee48208868304873b6d1b1ad691c743fa3accc5':
      // wq
      folder = 'programs/2c980fcd46b027cd64d75d974ee48208868304873b6d1b1ad691c743fa3accc5-wasm';
      break;
    case '3318c71ea11c4a759fa406bd5dec2038245d6b47c55c50b5127368d31949c6a3':
      // bfi
      folder = 'programs/3318c71ea11c4a759fa406bd5dec2038245d6b47c55c50b5127368d31949c6a3-wasm';
      break;
    case '5d913289af2f0ac09bca73b620e0bcc563327a94535494b9e0ca9e474cabff4c':
      // bf
      folder = 'programs/5d913289af2f0ac09bca73b620e0bcc563327a94535494b9e0ca9e474cabff4c-wasm'
      args = ['output.wasm']
      break;
    case 'b61f2422b9f7d490add208cfd8a53b7932f12140a5da01270e56c90b3f378996':
      // wasi-example
      folder = 'programs/b61f2422b9f7d490add208cfd8a53b7932f12140a5da01270e56c90b3f378996-wasm';
      args = ['-f']
      break;
    case 'cfa2c75ab461c6f7cdc228ba1c98e22b18bf0e7df637d54bb8f32a6abf703915':
      // save
      folder = 'programs/cfa2c75ab461c6f7cdc228ba1c98e22b18bf0e7df637d54bb8f32a6abf703915-wasm';
      break;
    case '5c8b3eea45224e5929738e4c5c3d1d1a43ec263a60dd102a96161de6d7012ef0':
      // rustpython
      folder = 'programs/5c8b3eea45224e5929738e4c5c3d1d1a43ec263a60dd102a96161de6d7012ef0-wasm';
      break;
    case '8d108de7e07ef1ee070ce2e1f057de253839d0be2c30a99f58889e85cf54449a':
      // handlebars-cli
      folder = 'programs/8d108de7e07ef1ee070ce2e1f057de253839d0be2c30a99f58889e85cf54449a-wasm';
      args = ['\'{"json": "data"}\''];
      break;
    case '310e582869203573dbbb7a06d8d5e8f9b457261f3b488bf2a37de85d227450ab':
      // clang.wasm
      folder = 'programs/310e582869203573dbbb7a06d8d5e8f9b457261f3b488bf2a37de85d227450ab-wasm';
      args = [];
      break;
    case 'a7a817db1c6c9fff26f005fd598cc28b15b35124927ec94822d8aa28cf824a7c':
      // libxml2.wasm
      folder = 'programs/a7a817db1c6c9fff26f005fd598cc28b15b35124927ec94822d8aa28cf824a7c-wasm';
      args = [];
      break;
    case '69286bc946266e89095151f5e12c77eae3824379b7b297141cc4cc7b5711832a':
      // canonicaljson.wasm
      folder = 'programs/69286bc946266e89095151f5e12c77eae3824379b7b297141cc4cc7b5711832a-wasm';
      args = [];
      break;
    case '0a0f242b77afe39fa5ff0cd579d7f6378b5869feae7ee792f416c9df19a5caf1':
      // zxing_barcode_reader.wasm
      folder = 'programs/0a0f242b77afe39fa5ff0cd579d7f6378b5869feae7ee792f416c9df19a5caf1-wasm';
      args = [];
      break;
    case '992205b67977f9c7a0d1ea7568d745e055fa3015d1ab70428feed6d8da60d060':
      // sqlite.wasm
      folder = 'programs/992205b67977f9c7a0d1ea7568d745e055fa3015d1ab70428feed6d8da60d060-wasm';
      args = [];
      break;
    case '7e48bfd68f658ac4916c291b536e6756c173f6ba63311c67c11ffa4b111f0bff':
      // viu.wasm
      folder = 'programs/7e48bfd68f658ac4916c291b536e6756c173f6ba63311c67c11ffa4b111f0bff-wasm';
      args = [];
      break;
    case '6f62a6bc5c8f8e3e12a54e2ecbc5674ccfe1c75f91d8e4dd6ebb3fec422a4d6c':
      // qjs.wasm
      folder = 'programs/6f62a6bc5c8f8e3e12a54e2ecbc5674ccfe1c75f91d8e4dd6ebb3fec422a4d6c-wasm';
      args = [];
      break;
    case 'f2d833c3da9a7b149d3d4e39eef8a091b8231cd71ece3e57c4ceafb2dd9714cd':
      // wasm-interface.wasm
      folder = 'programs/f2d833c3da9a7b149d3d4e39eef8a091b8231cd71ece3e57c4ceafb2dd9714cd-wasm';
      args = [];
      break;
    case '4165b9f433d19ee69c15ff5e87dac8ef7f04ce62be95c9a569a0a853048052de':
      // qjs.wasm
      folder = 'programs/4165b9f433d19ee69c15ff5e87dac8ef7f04ce62be95c9a569a0a853048052de-wasm';
      args = [];
      break;
    case '990040b990434ac4d48565af09f897c84ee02bf22f7e50e2091bb93b17a023db':
      // duk.wasm
      folder = 'programs/990040b990434ac4d48565af09f897c84ee02bf22f7e50e2091bb93b17a023db-wasm';
      args = [];
      break;
    case '722665633c07c06ea5ff8bab0c78a1bdad0998b06c769cb27efe9834d60bca3c':
      // jq.wasm
      folder = 'programs/722665633c07c06ea5ff8bab0c78a1bdad0998b06c769cb27efe9834d60bca3c-wasm';
      args = [];
      break;
    case 'bf750dfa5ccc0f32f59c865d6581acaae2f07feb1919429f2aafc11199206c66':
      // duk.wasm
      folder = 'programs/bf750dfa5ccc0f32f59c865d6581acaae2f07feb1919429f2aafc11199206c66-wasm';
      args = [];
      break;
    case 'ee42716385c8c8b1d9ba916fcf52ff2a258a4152f4e54af2262425d1832671fe':
      // qr2text.wasm
      folder = 'programs/ee42716385c8c8b1d9ba916fcf52ff2a258a4152f4e54af2262425d1832671fe-wasm';
      args = [];
      break;
    case '674a17949619b77f26560d8fd48ac65fb700ae6eed998ff74d62ce444c387d2f':
      // hq9_plus_rs.wasm
      folder = 'programs/674a17949619b77f26560d8fd48ac65fb700ae6eed998ff74d62ce444c387d2f-wasm';
      args = [];
      break;
    case 'a5a3bb5ef3d679f84b66655696c28415da174242e3bdae99030c1682156c5d04':
      // colcrt.wasm
      folder = 'programs/a5a3bb5ef3d679f84b66655696c28415da174242e3bdae99030c1682156c5d04-wasm';
      args = [];
      break;
    case '20cd76fa23f6760d8c4e94c861b93d3e316faaf7a464851a6b8b43db8ebb5cb6':
      // rev.wasm
      folder = 'programs/20cd76fa23f6760d8c4e94c861b93d3e316faaf7a464851a6b8b43db8ebb5cb6-wasm';
      args = [];
      break;
    case '65bb7368b55ebd366ce339cf58628750b203ea327e22c362f94dfa8bf8b55493':
      // basic.wasm
      folder = 'programs/65bb7368b55ebd366ce339cf58628750b203ea327e22c362f94dfa8bf8b55493-wasm';
      args = [];
    case '7e11233e9be20b791ca2584fb225a941357daa19b775a1dd16f18d57a2d21ab3':
      // cat.wasm
      folder = 'programs/7e11233e9be20b791ca2584fb225a941357daa19b775a1dd16f18d57a2d21ab3-wasm';
      args = [];
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
        res.native = nativeBinary ? (await runNativeBin(nativeBinary, crashFileTmp))[2] : NaN;
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
    stackCanaryCrashes,
    heapCanaryCrashes,
    crashes: count,
    inconclusiveCrashes: diffCount - stackCanaryCrashes - heapCanaryCrashes,
    lavaCrashes,
  };
}
