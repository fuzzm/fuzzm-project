#!/bin/bash
cp wasm-project/fuzzing_results/opj_compress_60h/findings-wasm.zip AFL-wasm/programs/opj_compress-wasm/
(cd AFL-wasm/programs/opj_compress-wasm/ && unzip -o findings-wasm.zip && cp -r opj_compress-wasm/findings/* findings)

cp wasm-project/fuzzing_results/libtiff_4d_new/findings-wasm.zip AFL-wasm/programs/pal2rgb-wasm/
(cd AFL-wasm/programs/pal2rgb-wasm/ && unzip -o findings-wasm.zip && cp -r findings/* findings)

cp wasm-project/fuzzing_results/pdfresurrect-dec-27/findings-wasm.zip AFL-wasm/programs/pdfresurrect-wasm/
(cd AFL-wasm/programs/pdfresurrect-wasm/ && unzip -o findings-wasm.zip && cp -r findings/* findings)

cp wasm-project/fuzzing_results/libpng_3h/findings-wasm.zip AFL-wasm/programs/pnm2png-wasm
(cd AFL-wasm/programs/pnm2png-wasm && unzip -o findings-wasm.zip && cp -r findings/* findings)

