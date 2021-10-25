# Overview
This project contains the Fuzzm WebAssembly fuzzer based on [AFL](https://github.com/google/AFL).

Here you can see it in action, fuzzing the motivating example from the paper:
![Fuzzm Screenshot](https://github.com/fuzzm/fuzzm-project/blob/master/screenshot.png?raw=true)

The structure of the folders is as follows:
 * [AFL-wasm](AFL-wasm) contains the adapted version of AFL, including the Wasmtime WebAssembly VM.
 * [wasm_instrumenter](wasm_instrumenter) contains the instrumentation tools for inserting (1) AFL-compatible coverage instrumentation and (2) stack and heap canaries. 
 * [benchmarks](benchmarks) contains the 10 benchmarks compiled from sources used in the evaluation of Fuzzm.
 * [wasm-bench](wasm-bench) contains the 18 WebAssembly binaries from the WasmBench dataset, used in the evaluation of Fuzzm, named by their SHA256 hash (which is used in WasmBench for indexing the associated metadata).
 * [benchmark-runner](benchmark-runner) contains a set of scripts for running the benchmarks, measuring the performance of the instrumentation, aggregating results from multiple runs and generating tables and plots for the paper.
 * [results](results) contains the results from running the benchmarks with both Fuzzm and AFL. This is also where the benchmark runner will store new results and where the aggregation script expects the results to be.
 * [plots](plots) contains the coverage- and crashes-over-time plots from Fuzzm, of which four were highlighted in the evaluation.
 * [poc-exploits](poc-exploits) contains the three proof-of-concept vulnerable WebAssembly binaries from [prior work](https://github.com/sola-st/wasm-binary-security/), and the binaries after instrumenting them with canaries.
 * [motivating-example](motivating-example) contains the full motivating example from the overview section, and instructions on how to compile it to native, native AFL, WebAssembly, and instrument it for Fuzzm.

# Requirements

* [Wasmtime](https://wasmtime.dev/) (tested with v0.20.0)
* [Rust and Cargo](https://doc.rust-lang.org/cargo/getting-started/installation.html)
* [npm and Node.js](https://nodejs.org/en/)
* Tested on Ubuntu 20.04

# Installation
`./init-workspace.sh`

Builds all subprojects and the benchmarks. 
