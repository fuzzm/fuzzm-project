#!/bin/bash
set -e 
git clone git@github.com:mtorp/magma-WASM.git magma
git clone git@github.com:mtorp/wasm-project.git
git clone git@github.com:mtorp/AFL-wasm.git
git clone git@github.com:mtorp/wasabi.git
git clone git@github.com:mtorp/LAVA-M.git
git clone git@github.com:mtorp/LAVA-native.git
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-11/wasi-sdk-11.0-linux.tar.gz
tar xvf wasi-sdk-11.0-linux.tar.gz
wget https://zlib.net/zlib-1.2.11.tar.gz
tar xvf zlib-1.2.11.tar.gz
