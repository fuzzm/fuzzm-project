//use walrus::ir::*;
extern crate clap;
use clap::{App, Arg};
use std::ffi::OsStr;
use std::process;

fn main() {
    let matches = App::new("Canaries")
        .version("1.0")
        .about("Inserts heap and stack canaries")
        .arg(
            Arg::with_name("INPUT")
                .help("Sets the input file to use")
                .required(true), //.index(0),
        )
        .arg(
            Arg::with_name("OUTPUT")
                .help("Sets the output file to use")
                .required(true), //.index(1),
        )
        .arg(
            Arg::with_name("skip-heap")
                .long("skip-heap")
                .help("don't instrument the file with heap canaries"),
        )
        .arg(
            Arg::with_name("skip-stack")
                .long("skip-stack")
                .help("don't instrument the file with stack canaries"),
        )
        .arg(
            Arg::with_name("skip-print")
                .long("skip-print")
                .help("don't instrument the file with print32 and print64 debug functions"),
        )
        .get_matches();

    //.arg(Arg::with_name("config")
    //     .short("c")
    //     .long("config")
    //     .value_name("FILE")
    //     .help("Sets a custom config file")
    //     .takes_value(true))
    //.arg(Arg::with_name("v")
    //     .short("v")
    //     .multiple(true)
    //     .help("Sets the level of verbosity"))
    //.subcommand(SubCommand::with_name("test")
    //            .about("controls testing features")
    //            .version("1.3")
    //            .author("Someone E. <someone_else@other.com>")
    //            .arg(Arg::with_name("debug")
    //                 .short("d")
    //                 .help("print debug information verbosely")))

    if let Err(e) = wasm_instrumenter::canaries::instrument_file(
        OsStr::new(matches.value_of("INPUT").unwrap()),
        OsStr::new(matches.value_of("OUTPUT").unwrap()),
        matches.is_present("skip-heap"),
        matches.is_present("skip-stack"),
        matches.is_present("skip-print"),
    ) {
        println!("Overflow detector failed with error: {}", e);
        process::exit(1);
    }
}
