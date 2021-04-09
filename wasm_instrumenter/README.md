This folder contains the tool for inserting [AFL](https://github.com/google/AFL)-compatible coverage extraction instrumentation, and the tool for inserting heap and stack canary checks.

# Building

`cargo build --release`

# Coverage instrumentation

`./target/release/afl_branch <INPUT> <OUTPUT>`

# Canaries

`./target/release/canaries <INPUT> <OUTPUT>`

use `--help` for additional options.

