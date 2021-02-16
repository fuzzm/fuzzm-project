const path = require('path');
const cp = require('child_process');
const fs = require('fs');
const commander = require('commander');

commander
  .arguments('')
  .description('copies the benchmarks into the AFL folder')
  .option('-n, --skip-seeds', 'does not copy over the seeds')
  .option('-n, --skip-dics', 'does not copy over the dictionaries')
  .action(function (options) {
    const scriptDir = __dirname;
    const aflFolder = path.resolve(scriptDir, './AFL-wasm')
    const instrumenterFolder = path.resolve(scriptDir, './wasm-project/wasm_instrumenter');

    if (options.skipSeeds) {
      console.log('skipping copy of benchmark seeds');
    }

    if (options.skipDics) {
      console.log('skipping copy of benchmark dictionaries');
    }

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

      if (seedSrcFolder && !options.skipSeeds) {
        cp.execSync(`cp ${seedSrcFolder}/* ${seedFolder}`);
      }

      if (dictionarySrcFolder && !options.skipDics) {
        cp.execSync(`cp ${dictionarySrcFolder} ${path.resolve(dicFolder, 'd.dict')}`);
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

    /////// LAVA WASM ///////

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

    /////// LAVA native ///////

    const LavaNativeDir = 'LAVA-native';

    const base64Native = path.resolve(LavaNativeDir, 'base64', 'coreutils-8.24-lava-safe');
    const base64NativeFuzzerInput = path.resolve(LavaNativeDir, 'base64', 'fuzzer_input');
    prepareBenchmark(base64Native, "base64", base64NativeFuzzerInput);

    const md5Native=path.resolve(LavaNativeDir, 'md5sum', 'coreutils-8.24-lava-safe');
    const md5NativeFuzzerInput=path.resolve(LavaNativeDir, 'md5sum', 'fuzzer_input');
    prepareBenchmark(md5Native, "md5sum", md5NativeFuzzerInput);

    const uniqNative=path.resolve(LavaNativeDir, 'uniq', 'coreutils-8.24-lava-safe');
    const uniqNativeFuzzerInput=path.resolve(LavaNativeDir, 'uniq', 'fuzzer_input');
    prepareBenchmark(uniqNative, "uniq", uniqNativeFuzzerInput);

    /////// vulnerable benchmarks ///////

    const VulnBenchDir = 'wasm-project/benchmarks';
    const pdfResurrect=path.resolve(VulnBenchDir, 'pdfresurrect');
    const pdfResurrectFuzzerInput=path.resolve(aflFolder, 'testcases/others/pdf');
    const pdfResurrectDictionary=path.resolve(aflFolder, 'dictionaries/pdf.dict');
    prepareBenchmark(pdfResurrect, 'pdfresurrect.wasm', pdfResurrectFuzzerInput, pdfResurrectDictionary);
    prepareBenchmark(pdfResurrect, 'pdfresurrect', pdfResurrectFuzzerInput, pdfResurrectDictionary);

    const openjpeg=path.resolve(VulnBenchDir, 'openjpeg/bin');
    const openjpegFuzzerInput=path.resolve(aflFolder, 'testcases/images/bmp');
    // const openjpegDictionary=path.resolve(aflFolder, 'dictionaries/bmp.dict');
    prepareBenchmark(openjpeg, 'opj_compress.wasm', openjpegFuzzerInput);
    prepareBenchmark(openjpeg, 'opj_compress', openjpegFuzzerInput);

    const tiff = path.resolve(VulnBenchDir, 'tiff-4.0.9/tools');
    const tiffFuzzerInput=path.resolve(aflFolder, 'testcases/images/tiff');
    const tiffDictionary=path.resolve(aflFolder, 'dictionaries/tiff.dict');
    prepareBenchmark(tiff, 'pal2rgb.wasm', tiffFuzzerInput, tiffDictionary);
    prepareBenchmark(tiff, 'pal2rgb', tiffFuzzerInput, tiffDictionary);

    png=path.resolve(VulnBenchDir, 'libpng/contrib/pngminus');
    pngFuzzerInput=path.resolve(aflFolder, 'testcases/images/pnm');
    prepareBenchmark(png, 'pnm2png.wasm', pngFuzzerInput);
    prepareBenchmark(png, 'pnm2png', pngFuzzerInput);
  }).parse(process.argv);
