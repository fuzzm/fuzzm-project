#!/bin/bash
# wasm
(cd LAVA-M/base64/coreutils-8.24-lava-safe && ./wasm-compile)
(cd LAVA-M/md5sum/coreutils-8.24-lava-safe && ./wasm-compile)
(cd LAVA-M/uniq/coreutils-8.24-lava-safe && ./wasm-compile)

# native
(cd LAVA-native/base64/coreutils-8.24-lava-safe && ./compile.sh)
(cd LAVA-native/md5sum/coreutils-8.24-lava-safe && ./compile.sh)
(cd LAVA-native/uniq/coreutils-8.24-lava-safe && ./compile.sh)
