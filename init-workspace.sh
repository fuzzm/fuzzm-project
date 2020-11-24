#!/bin/bash
set -e 
git clone git@github.com:mtorp/magma-WASM.git
git clone git@github.com:mtorp/wasm-project.git
git clone git@github.com:mtorp/AFL-wasm.git
git clone git@github.com:mtorp/wasabi.git
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-11/wasi-sdk-11.0-linux.tar.gz
tar xvf wasi-sdk-11.0-linux.tar.gz
