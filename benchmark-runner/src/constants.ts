import { resolve } from 'path';

const BENCHMARK_RUNNER_FOLDER = resolve(__dirname, '../');
export const PROJECT_FOLDER = resolve(BENCHMARK_RUNNER_FOLDER, '../');
export const AFL_FOLDER = resolve(PROJECT_FOLDER, 'AFL-wasm');
export const WASM_BENCHMARK_SCRIPT = resolve(AFL_FOLDER, 'wasm-benchmark.sh');
export const NATIVE_BENCHMARK_SCRIPT = resolve(AFL_FOLDER, 'native-benchmark.sh');
export const RESULTS_DIR = resolve(PROJECT_FOLDER, 'results');
export const BENCHMARKS_FOLDER = resolve(PROJECT_FOLDER, 'benchmarks');
export const LAVA_BENCHMARKS_FOLDER = resolve(BENCHMARKS_FOLDER, 'LAVA-M');
export const INSTRUMENTER_DIST_FOLDER = resolve(PROJECT_FOLDER, 'wasm_instrumenter', 'target', 'release');
export const CANARIES_INSTRUMENTER_BIN = resolve(INSTRUMENTER_DIST_FOLDER, 'canaries');
export const AFL_INSTRUMENTER_BIN = resolve(INSTRUMENTER_DIST_FOLDER, 'afl_branch');
export const PLOT_FOLDER = resolve(BENCHMARK_RUNNER_FOLDER, 'plots');
//export const PAPER_FOLDER = resolve(BENCHMARK_RUNNER_FOLDER, '../', 'paper');
//export const PAPER_FIGURE_FOLDER = resolve(PAPER_FOLDER, 'figs');

interface Benchmark {
  name: string;
  cmd: string;
  dictionary?: string;
  // custom input file for AFL (-f option)
  // useful when input files must have a specific extension
  inputFile?: string;
  // can be viewed as a form of comment
  originalName?: string;
  // can be viewed as a form of comment
  problems?: string;
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
    dictionary: 'programs/pal2rgb-wasm/dictionary/d.dict',
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
  '310e582869203573dbbb7a06d8d5e8f9b457261f3b488bf2a37de85d227450ab': {
    originalName: 'clang.wasm',
    name: '310e582869203573dbbb7a06d8d5e8f9b457261f3b488bf2a37de85d227450ab-wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get',
  },
  '5c8b3eea45224e5929738e4c5c3d1d1a43ec263a60dd102a96161de6d7012ef0': {
    name: '5c8b3eea45224e5929738e4c5c3d1d1a43ec263a60dd102a96161de6d7012ef0-wasm',
    originalName: 'rustpython.wasm',
    cmd: '@@',
    problems: 'none'
  },
  '8d108de7e07ef1ee070ce2e1f057de253839d0be2c30a99f58889e85cf54449a': {
    name: '8d108de7e07ef1ee070ce2e1f057de253839d0be2c30a99f58889e85cf54449a-wasm',
    originalName: 'handlebars-cli.wasm',
    cmd: '@@ \'{"json": "data"}\'',
    problems: 'does not make progress. Probably because fuzzer cannot guess JSON input'
  },
  'a7a817db1c6c9fff26f005fd598cc28b15b35124927ec94822d8aa28cf824a7c': {
    name: 'a7a817db1c6c9fff26f005fd598cc28b15b35124927ec94822d8aa28cf824a7c-wasm',
    originalName: 'libxml2.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '69286bc946266e89095151f5e12c77eae3824379b7b297141cc4cc7b5711832a': {
    name: '69286bc946266e89095151f5e12c77eae3824379b7b297141cc4cc7b5711832a-wasm',
    originalName: 'canonicaljson.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '0a0f242b77afe39fa5ff0cd579d7f6378b5869feae7ee792f416c9df19a5caf1': {
    name: '0a0f242b77afe39fa5ff0cd579d7f6378b5869feae7ee792f416c9df19a5caf1-wasm',
    originalName: 'zxing_barcode_reader.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '992205b67977f9c7a0d1ea7568d745e055fa3015d1ab70428feed6d8da60d060': {
    name: '992205b67977f9c7a0d1ea7568d745e055fa3015d1ab70428feed6d8da60d060-wasm',
    originalName: 'sqlite.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '7e48bfd68f658ac4916c291b536e6756c173f6ba63311c67c11ffa4b111f0bff': {
    name: '7e48bfd68f658ac4916c291b536e6756c173f6ba63311c67c11ffa4b111f0bff-wasm',
    originalName: 'viu.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '94a407dd35917784d50cac9d7bf58ea579b400a0d4a07e6b1834107520714b3c': {
    name: '94a407dd35917784d50cac9d7bf58ea579b400a0d4a07e6b1834107520714b3c-wasm',
    originalName: 'sane-fmt.wasm',
    cmd: '@@',
    problems: 'crashes for unknown reason'
  },
  '6f62a6bc5c8f8e3e12a54e2ecbc5674ccfe1c75f91d8e4dd6ebb3fec422a4d6c': {
    name: '6f62a6bc5c8f8e3e12a54e2ecbc5674ccfe1c75f91d8e4dd6ebb3fec422a4d6c-wasm',
    originalName: 'qjs.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  'f2d833c3da9a7b149d3d4e39eef8a091b8231cd71ece3e57c4ceafb2dd9714cd': {
    name: 'f2d833c3da9a7b149d3d4e39eef8a091b8231cd71ece3e57c4ceafb2dd9714cd-wasm',
    originalName: 'wasm-interface.wasm',
    cmd: 'check @@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '4165b9f433d19ee69c15ff5e87dac8ef7f04ce62be95c9a569a0a853048052de': {
    name: '4165b9f433d19ee69c15ff5e87dac8ef7f04ce62be95c9a569a0a853048052de-wasm',
    originalName: 'qjs.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  'b61f2422b9f7d490add208cfd8a53b7932f12140a5da01270e56c90b3f378996': {
    name: 'b61f2422b9f7d490add208cfd8a53b7932f12140a5da01270e56c90b3f378996-wasm',
    originalName: 'wasi-example.wasm',
    cmd: '-f @@',
    problems: 'none, and looks promising. It finds crashes during quick 10 min run'
  },
  '2c980fcd46b027cd64d75d974ee48208868304873b6d1b1ad691c743fa3accc5': {
    name: '2c980fcd46b027cd64d75d974ee48208868304873b6d1b1ad691c743fa3accc5-wasm',
    originalName: 'wq.wasm',
    cmd: '@@',
    problems: 'none'
  },
  '3318c71ea11c4a759fa406bd5dec2038245d6b47c55c50b5127368d31949c6a3': {
    name: '3318c71ea11c4a759fa406bd5dec2038245d6b47c55c50b5127368d31949c6a3-wasm',
    originalName: 'bfi.wasm',
    cmd: '@@',
    problems: 'none'
  },
  '5d913289af2f0ac09bca73b620e0bcc563327a94535494b9e0ca9e474cabff4c': {
    name: '5d913289af2f0ac09bca73b620e0bcc563327a94535494b9e0ca9e474cabff4c-wasm',
    originalName: 'bf.wasm',
    cmd: '@@ output.wasm',
    problems: 'none'
  },
  'cfa2c75ab461c6f7cdc228ba1c98e22b18bf0e7df637d54bb8f32a6abf703915': {
    name: 'cfa2c75ab461c6f7cdc228ba1c98e22b18bf0e7df637d54bb8f32a6abf703915-wasm',
    originalName: 'save.wasm',
    cmd: '@@',
    problems: 'none'
  },
  '990040b990434ac4d48565af09f897c84ee02bf22f7e50e2091bb93b17a023db': {
    name: '990040b990434ac4d48565af09f897c84ee02bf22f7e50e2091bb93b17a023db-wasm',
    originalName: 'duk.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '722665633c07c06ea5ff8bab0c78a1bdad0998b06c769cb27efe9834d60bca3c': {
    name: '722665633c07c06ea5ff8bab0c78a1bdad0998b06c769cb27efe9834d60bca3c-wasm',
    originalName: 'jq.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  'bf750dfa5ccc0f32f59c865d6581acaae2f07feb1919429f2aafc11199206c66': {
    name: 'bf750dfa5ccc0f32f59c865d6581acaae2f07feb1919429f2aafc11199206c66-wasm',
    originalName: 'duk.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  'f49ceb352984feff82d1f3dd1f6cbeedd2e62cfcdb5e9c0289d924ad7c8fd7bf': {
    name: 'f49ceb352984feff82d1f3dd1f6cbeedd2e62cfcdb5e9c0289d924ad7c8fd7bf-wasm',
    originalName: 'rsign.wasm',
    cmd: 'sign @@',
    problems: 'none but has only two paths'
  },
  'ee42716385c8c8b1d9ba916fcf52ff2a258a4152f4e54af2262425d1832671fe': {
    name: 'ee42716385c8c8b1d9ba916fcf52ff2a258a4152f4e54af2262425d1832671fe-wasm',
    originalName: 'qr2text.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '674a17949619b77f26560d8fd48ac65fb700ae6eed998ff74d62ce444c387d2f': {
    name: '674a17949619b77f26560d8fd48ac65fb700ae6eed998ff74d62ce444c387d2f-wasm',
    originalName: 'hq9_plus_rs.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  'a5a3bb5ef3d679f84b66655696c28415da174242e3bdae99030c1682156c5d04': {
    name: 'a5a3bb5ef3d679f84b66655696c28415da174242e3bdae99030c1682156c5d04-wasm',
    originalName: 'colcrt.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '20cd76fa23f6760d8c4e94c861b93d3e316faaf7a464851a6b8b43db8ebb5cb6': {
    name: '20cd76fa23f6760d8c4e94c861b93d3e316faaf7a464851a6b8b43db8ebb5cb6-wasm',
    originalName: 'rev.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '65bb7368b55ebd366ce339cf58628750b203ea327e22c362f94dfa8bf8b55493': {
    name: '65bb7368b55ebd366ce339cf58628750b203ea327e22c362f94dfa8bf8b55493-wasm',
    originalName: 'basic.wasm',
    cmd: '@@',
    problems: 'wasi_unstable::fd_prestat_get'
  },
  '7e11233e9be20b791ca2584fb225a941357daa19b775a1dd16f18d57a2d21ab3': {
    name: '7e11233e9be20b791ca2584fb225a941357daa19b775a1dd16f18d57a2d21ab3-wasm',
    originalName: 'cat.wasm',
    cmd: '@@',
    problems: 'none but only one path'
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
