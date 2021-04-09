# Overview
This project contains the Fuzzm WebAssembly fuzzer based on [AFL](https://github.com/google/AFL).

The structure of the folders is as follows:
 * [AFL-wasm](AFL-wasm) contains the ported version of AFL. 
 * [wasm_instrumenter](wasm_instrumenter) contains the instrumentation tools for inserting (1) AFL-compatible coverage instrumentation and (2) stack and heap canaries. 
 * [bencmarks](benchmarks) contains the 10 benchmarks used in the evaluation of Fuzzm.
 * [benchmark-runner](benchmark-runner) contains a set of scripts for running the benchmarks, measuring performance of the instrumentation, aggregating results over multiple runs and generating tables and plots for the paper.
 * [results](results) contains the results from running the 10 benchmarks with both Fuzzm and AFL. This is also where the benchmark runner will store new results and where the aggregation script expect the results to be.

# Requirements

* [Wasmtime](https://wasmtime.dev/) (tested with v0.20.0)
* [Rust and Cargo](https://doc.rust-lang.org/cargo/getting-started/installation.html)
* [npm and Node.js](https://nodejs.org/en/)
* Tested on Ubuntu 20.04

# Installation
`./init-workspace.sh`



