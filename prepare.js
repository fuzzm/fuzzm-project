const path = require('path');
const cp = require('child_process');
const fs = require('fs');

const scriptDir = __dirname;
const aflFolder = path.resolve(scriptDir, './AFL-wasm')
const instrumenterFolder = path.resolve(scriptDir, './wasm-project/wasm_instrumenter');

cp.execSync('make', {cwd: aflFolder});
cp.execSync('cargo build --release', {cwd: instrumenterFolder});

function prepareBenchmark(benchmarkSrcFolder,
  binaryName,
  seedSrcFolder,
  dictionarySrcFolder) {
  console.log(`processing ${binaryName}`);
  const wasmMode = binaryName.endsWith(".wasm");
  const dst = path.resolve(aflFolder, 'programs', binaryName.replace(".wasm", "-wasm"));
  const seedFolder = path.resolve(dst, 'test-case');
  const dicFolder = path.resolve(dst, 'dictionary');
  const findingsFolder = path.resolve(dst, 'findings');

  fs.rmdirSync(dst, {recursive: true});
  fs.mkdirSync(findingsFolder, {recursive: true});
  fs.mkdirSync(seedFolder, {recursive: true});
  fs.mkdirSync(dicFolder, {recursive: true});

  if (seedSrcFolder) {
    cp.execSync(`cp ${seedSrcFolder}/* ${seedFolder}`);
  }

  if (dictionarySrcFolder) {
    cp.execSync(`cp ${dictionarySrcFolder}/* ${path.resolve(dicFolder, 'd.dict')}`);
  }

  const binDst = path.resolve(dst, wasmMode ? 'prog.wasm' : 'prog');
  cp.execSync(`cp ${path.resolve(benchmarkSrcFolder, binaryName)} ${binDst}`);

  if (wasmMode) {
    const instrBinFolder = path.resolve(instrumenterFolder, 'target', 'release');
    const aflBranch = path.resolve(instrBinFolder, 'afl_branch');
    const canaries = path.resolve(instrBinFolder, 'canaries');
    const instrBinDst = `${binDst}.instr`;
    const aflBranchOutput = cp.execSync(`${aflBranch} ${binDst} ${instrBinDst}`, {encoding: 'utf-8'});
    console.log(aflBranchOutput);
    cp.execSync(`${aflBranch} ${instrBinDst} ${instrBinDst} --skip-print`);
    cp.execSync(`chmod +x ${instrBinDst}`);
  }
}

const LavaDir="LAVA-M";
const base64Wasm=path.resolve(LavaDir, 'base64', 'coreutils-8.24-lava-safe');
const base64FuzzerInput=path.resolve(LavaDir, 'base64', 'fuzzer_input');
prepareBenchmark(base64Wasm, 'base64.wasm', base64FuzzerInput);

const md5Wasm=path.resolve(LavaDir, 'md5sum', 'coreutils-8.24-lava-safe');
const md5FuzzerInput=path.resolve(LavaDir, 'md5sum', 'fuzzer_input');
prepareBenchmark(md5Wasm, "md5sum.wasm", md5FuzzerInput);

const uniqWasm=path.resolve(LavaDir, 'uniq', 'coreutils-8.24-lava-safe');
const uniqFuzzerInput=path.resolve(LavaDir, 'uniq', 'fuzzer_input');
prepareBenchmark(uniqWasm, "uniq.wasm", uniqFuzzerInput);
