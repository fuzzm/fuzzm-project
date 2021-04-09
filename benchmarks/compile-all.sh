#!/bin/bash
(cd libpng && ./compile.sh && ./wasm-compile.sh)
(cd openjpeg && ./compile.sh && ./wasm-compile.sh)
(cd pdfresurrect && ./compile.sh && ./wasm-compile.sh)
(cd tiff-4.0.9 && ./compile.sh && ./wasm-compile.sh)
(cd abc2mtex && ./compile.sh && ./wasm-compile.sh)
(cd flac && ./compile.sh && ./wasm-compile.sh)
(cd jbig2dec && ./compile.sh && ./wasm-compile.sh)
./compile-lava.sh
