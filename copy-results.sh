#!/bin/bash
cp wasm-project/fuzzing_results/opj_compress_54h/findings-wasm.zip AFL-wasm/programs/opj_compress-wasm/
(cd AFL-wasm/programs/opj_compress-wasm/ && unzip -o findings-wasm.zip && cp -r opj_compress-wasm/findings/* findings)

cp wasm-project/fuzzing_results/libtiff_2d/findings.zip AFL-wasm/programs/pal2rgb-wasm/
(cd AFL-wasm/programs/pal2rgb-wasm/ && unzip -o findings.zip && cp -r findings/* findings)

cp wasm-project/fuzzing_results/pdfresurrect-dec-27/findings-wasm.zip AFL-wasm/programs/pdfresurrect-wasm/
(cd AFL-wasm/programs/pdfresurrect-wasm/ && unzip -o findings-wasm.zip && cp -r findings/* findings)
