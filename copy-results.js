#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const cp = require('child_process');

const resultsFolder = "./results";
const dstFolders = "./AFL-wasm/programs"

fs.readdir(resultsFolder, {withFileTypes: true}, (err, files) => {
  const benchmarkDirectories = files.filter(f => f.isDirectory());

  for (const b of benchmarkDirectories) {
    console.log(`processing ${b.name}`);

    const bResults = fs.readdir(path.resolve(resultsFolder, b.name), {withFileTypes: true}, (err, files) => {
      const resultFiles = files.filter(f => f.name.endsWith("zip"));
      const dateRegexp = /(\d+)-(\d+)-(\d+)_(\d+):(\d+)/

      const ds = []; 

      for (const f of resultFiles) {
        const m = f.name.match(dateRegexp);
        if (m) {
          const day = m[1] > 9 ? m[1] : `0${m[1]}`;
          const month = m[2] > 9 ? m[2] : `0${m[2]}`;
          const year = m[3];
          const hour = m[4] > 9 ? m[4] : `0${m[4]}`;
          const min = m[5] > 9 ? m[5] : `0${m[5]}`;
          const isoDateStr = `${year}-${month}-${day}T${hour}:${min}:00.000Z`;
          const date = Date.parse(isoDateStr);
          ds.push({f, date});
        }
      }
      // newest first
      ds.sort((a, b) => b.date - a.date); 

      if (!ds.length) {
        console.error(`empty results directory for ${b.name}`);
        return;
      }

      const zipName = ds[0].f.name;
      const zipSource = path.resolve(resultsFolder, b.name, zipName);

      // remove -native from native benchmarks;
      const dstName = b.name.includes("wasm") ? b.name : b.name.substring(0, b.name.indexOf('-native'));
      const dstFolder = path.resolve(dstFolders, dstName);

      // create dst directory
      fs.mkdirSync(dstFolder, {recursive: true});

      // copy zip 
      const zipDst = path.resolve(dstFolder, zipName);
      fs.copyFileSync(zipSource, zipDst);

      // remove old findings
      const findingsFolder = path.resolve(dstFolder, 'findings');
      fs.rmdirSync(findingsFolder, {recursive: true});
      fs.mkdirSync(findingsFolder, {recursive: true});

      // unzip zip
      const stdout = cp.execSync(`unzip -o ${zipName}`, {cwd: dstFolder, stdio: 'ignore'});

      // copy to findings
      // TODO always use repeat-0? 
      const unzippedContents = path.resolve(dstFolder, path.basename(zipName, ".zip"), 'repeat-0');
      cp.execSync(`cp -r ${unzippedContents}/* ${findingsFolder}`, {stdio: 'ignore'});

      if (b.name.includes('opj_compress')) {
        // add the .bmp extension to all crash files
        const crashFolder = path.resolve(findingsFolder, 'crashes');
        fs.readdir(crashFolder,  {withFileTypes: true}, (err, files) => {
          files.forEach(f => {
            const fp = path.resolve(crashFolder, f.name);
            const fpNew = `${fp}.bmp`;
            fs.copyFileSync(fp, fpNew);
            fs.unlinkSync(fp);
          });
        });
      }

      if (b.name.includes('abc2mtex')) {
        // add the rename files to crash-idx since abc2mtex doesn't like : in file names
        const crashFolder = path.resolve(findingsFolder, 'crashes');
        fs.readdir(crashFolder,  {withFileTypes: true}, (err, files) => {
          files.forEach((f, idx) => {
            const fp = path.resolve(crashFolder, f.name);
            const fpNew = path.resolve(crashFolder, `crash-${idx}`);
            fs.copyFileSync(fp, fpNew);
            fs.unlinkSync(fp);
          });
        });
      }
    });
  }
});
