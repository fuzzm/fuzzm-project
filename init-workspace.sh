#!/bin/bash
set -e 

# download wasi sdk
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-11/wasi-sdk-11.0-linux.tar.gz
tar xvf wasi-sdk-11.0-linux.tar.gz

# install prepare.js and copy-results.js dependencies
npm install

# compile AFL-wasm
(cd AFL-wasm && make)

# compile wasm_instrumenter
(cd wasm_instrumenter && cargo build --release)

# compile benchmark-runner
(cd benchmark-runner && npm i && npm run build)

#compile benchmarks
(cd benchmarks && ./compile-all.sh)
