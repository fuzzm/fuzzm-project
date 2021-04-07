import { resolve } from 'path';

const BENCHMARK_RUNNER_FOLDER = resolve(__dirname, '../');
export const PROJECT_FOLDER = resolve(BENCHMARK_RUNNER_FOLDER, '../');
export const AFL_FOLDER = resolve(PROJECT_FOLDER, 'AFL-wasm');
export const WASM_BENCHMARK_SCRIPT = resolve(AFL_FOLDER, 'wasm-benchmark.sh');
export const NATIVE_BENCHMARK_SCRIPT = resolve(AFL_FOLDER, 'native-benchmark.sh');
export const RESULTS_DIR = resolve(BENCHMARK_RUNNER_FOLDER, 'results');
export const BENCHMARKS_FOLDER = resolve(PROJECT_FOLDER, 'benchmarks');
export const INSTRUMENTER_DIST_FOLDER = resolve(PROJECT_FOLDER, 'wasm_instrumenter', 'target', 'release');
export const CANARIES_INSTRUMENTER_BIN = resolve(INSTRUMENTER_DIST_FOLDER, 'canaries');
export const AFL_INSTRUMENTER_BIN = resolve(INSTRUMENTER_DIST_FOLDER, 'afl_branch');
export const PAPER_FOLDER = resolve(BENCHMARK_RUNNER_FOLDER, '../', 'paper');
export const PAPER_FIGURE_FOLDER = resolve(PAPER_FOLDER, 'figs');

interface Benchmark {
  name: string;
  cmd: string;
  dictionary?: string;
  // custom input file for AFL (-f option)
  // useful when input files must have a specific extension
  inputFile?: string;
}

export const FUZZ_COMMANDS_WASM: { [index: string]: Benchmark } = {
  base64: {
    name: 'base64-wasm',
    cmd: '--decode @@',
  },
  md5sum: {
    name: 'md5sum-wasm',
    cmd: '-c @@',
  },
  opj_compress: {
    name: 'opj_compress-wasm',
    cmd: `-i ${AFL_FOLDER}/programs/opj_compress-wasm/input.bmp -o ${AFL_FOLDER}/programs/opj_compress-wasm/out.jp2`,
    inputFile: 'programs/opj_compress-wasm/input.bmp',
  },
  pal2rgb: {
    name: 'pal2rgb-wasm',
    cmd: '@@ /dev/null',
    dictionary: 'programs/pdfresurrect-wasm/dictionary/d.dict',
  },
  pdfresurrect: {
    name: 'pdfresurrect-wasm',
    cmd: '@@',
    dictionary: 'programs/pdfresurrect-wasm/dictionary/d.dict',
  },
  uniq: {
    name: 'uniq-wasm',
    cmd: '@@',
  },
  pnm2png: {
    name: 'pnm2png-wasm',
    inputFile: 'programs/pnm2png-wasm/input.pnm',
    cmd: `${AFL_FOLDER}/programs/pnm2png-wasm/input.pnm /dev/null`,
  },
  abc2mtex: {
    name: 'abc2mtex-wasm',
    inputFile: 'programs/abc2mtex-wasm/input.abc',
    cmd: `programs/abc2mtex-wasm/input.abc`,
  },
  flac: {
    name: 'flac-wasm',
    //force and decode
    cmd: `-f -d @@`,
  },
  jbig2dec: {
    name: 'jbig2dec-wasm',
    // output type should be png (don' think that is crucial)
    cmd: `@@ -t png -o /dev/null`,
  },
};

export const FUZZ_COMMANDS_NATIVE: { [index: string]: Benchmark } = {
  base64: {
    name: 'base64',
    cmd: '--decode @@',
  },
  md5sum: {
    name: 'md5sum',
    cmd: '-c @@',
  },
  opj_compress: {
    name: 'opj_compress',
    inputFile: 'programs/opj_compress/input.bmp',
    cmd: '-i programs/opj_compress/input.bmp -o programs/opj_compress/out.jp2',
  },
  pal2rgb: {
    name: 'pal2rgb',
    dictionary: 'programs/pal2rgb/dictionary/d.dict',
    cmd: '@@ /dev/null',
  },
  pdfresurrect: {
    name: 'pdfresurrect',
    dictionary: 'programs/pdfresurrect/dictionary/d.dict',
    cmd: '@@',
  },
  uniq: {
    name: 'uniq',
    cmd: '@@',
  },
  pnm2png: {
    name: 'pnm2png',
    inputFile: 'programs/pnm2png/input.pnm',
    cmd: 'programs/pnm2png/input.pnm /dev/null',
  },
  abc2mtex: {
    name: 'abc2mtex',
    cmd: `@@`,
  },
  flac: {
    name: 'flac',
    //force and decode
    cmd: `-f -d @@`,
  },
  jbig2dec: {
    name: 'jbig2dec',
    // output type should be png (don' think that is crucial)
    cmd: `-t png -o /dev/null @@`,
  },
};
