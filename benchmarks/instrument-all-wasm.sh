#!/bin/bash
(cd libpng && ./wasm-compile-and-instrument.sh)
(cd openjpeg && ./wasm-compile-and-instrument.sh)
(cd pdfresurrect && ./wasm-compile-and-instrument.sh)
(cd tiff-4.0.9 && ./wasm-compile-and-instrument.sh)
(cd abc2mtex && ./wasm-compile-and-instrument.sh)
(cd flac && ./wasm-compile-and-instrument.sh)
(cd jbig2dec && ./wasm-compile-and-instrument.sh)
