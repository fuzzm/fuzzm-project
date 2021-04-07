use std::error::Error;
use std::ffi::OsStr;
use std::io::Result;
use std::path::PathBuf;
use std::process::{Command, ExitStatus, Stdio};
use std::{ffi, result};
use serial_test::serial;

static BENCHMARKS_LOC: &'static str = "benchmarks";

fn run_test<T>(
    benchmark_name: &str,
    input_file_name: &str,
    instrumentation_func: T,
) -> result::Result<(), Box<dyn Error>>
where
    T: Fn(&OsStr, &OsStr) -> result::Result<(), Box<dyn Error>>,
{
    let benchmark_folder: PathBuf = [BENCHMARKS_LOC, benchmark_name].iter().collect();
    let mut compile_script = benchmark_folder.clone();
    compile_script.push("compile.sh");

    run_program(OsStr::new("sh"), &[compile_script.as_os_str()])?;

    let mut wasm_module = benchmark_folder.clone();
    wasm_module.push(format!("{}.wasm", benchmark_name));

    instrumentation_func(wasm_module.as_os_str(), wasm_module.as_os_str())?;

    let mut js_prog = benchmark_folder.clone();
    js_prog.push(format!("{}.js", benchmark_name));

    let mut input_file = benchmark_folder.clone();
    input_file.push(input_file_name);

    let res = run_program(
        OsStr::new("node"),
        &[js_prog.as_os_str(), input_file.as_os_str()],
    )?;

    let mut clean_script = benchmark_folder.clone();
    clean_script.push("clean.sh");
    run_program(OsStr::new("sh"), &[clean_script.as_os_str()])?;

    println!("exit code!?!?! {}", res.code().unwrap_or(-1));

    match res.code() {
        Some(0) => Ok(()),
        Some(n) => Err(format!("program exited with a non-zero exit code {}", n).into()),
        None => Err(String::from("could not extract error code from run of program").into()),
    }
}

#[test]
#[serial]
fn gzip_canaries() -> result::Result<(), Box<dyn Error>> {
    run_test(
        "gzip",
        "file.txt",
        wasm_instrumenter::canaries::instrument_file,
    )
}

#[test]
#[serial]
fn bzip2_canaries() -> result::Result<(), Box<dyn Error>> {
    run_test(
        "bzip2",
        "file.txt",
        wasm_instrumenter::canaries::instrument_file,
    )
}

#[test]
#[serial]
fn oggenc_canaries() -> result::Result<(), Box<dyn Error>> {
    run_test(
        "oggenc",
        "jfk_1963_0626_berliner.wav",
        wasm_instrumenter::canaries::instrument_file,
    )
}

#[test]
#[serial]
fn gzip_afl_instrumentation() -> result::Result<(), Box<dyn Error>> {
    run_test(
        "gzip",
        "file.txt",
        wasm_instrumenter::afl_branch::instrument_file,
    )
}

#[test]
#[serial]
fn bzip2_afl_instrumentation() -> result::Result<(), Box<dyn Error>> {
    run_test(
        "bzip2",
        "file.txt",
        wasm_instrumenter::afl_branch::instrument_file,
    )
}

#[test]
#[serial]
fn oggenc_afl_instrumentation() -> result::Result<(), Box<dyn Error>> {
    run_test(
        "oggenc",
        "jfk_1963_0626_berliner.wav",
        wasm_instrumenter::afl_branch::instrument_file,
    )
}


fn run_program(program: &ffi::OsStr, args: &[&ffi::OsStr]) -> Result<ExitStatus> {
    // use Stdio::null() to suppress the output.
    Command::new(program)
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .args(args)
        .spawn()?
        .wait()
}
