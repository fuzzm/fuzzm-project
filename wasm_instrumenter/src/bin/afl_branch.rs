use std::ffi::OsStr;

fn main() {
    let args = &(std::env::args().collect::<Vec<_>>())[1..3];
    let input = OsStr::new(&args[0]);
    let output = OsStr::new(&args[1]);

    wasm_instrumenter::afl_branch::instrument_file(input, output)
        .expect(format!("could not instrument file {}", input.to_str().unwrap()).as_str());
}
