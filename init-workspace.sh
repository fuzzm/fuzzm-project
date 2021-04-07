#!/bin/bash
set -e 
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-11/wasi-sdk-11.0-linux.tar.gz
tar xvf wasi-sdk-11.0-linux.tar.gz
npm install
