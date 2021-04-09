use crate::shared::find_func_by_name;
use rand::distributions::{Distribution, Uniform};
use rand::rngs::ThreadRng;
use std::error::Error;
use std::ffi::OsStr;
use wasm::highlevel::{
    Global, GlobalOp, ImportOrPresent, Instr, LoadOp, Local, LocalOp, Module, NumericOp, StoreOp,
};
use wasm::{FunctionType, Idx, Label, Memarg, Mutability, Val, ValType};

/// must be kept up-to-date with the AFL implementation
const MAP_SIZE_POW2: u32 = 16;
const MAP_SIZE: u32 = 1 << MAP_SIZE_POW2;

/// Inserts code for allocating the shared memory heap chunk (64 kB)
///
/// Inserts the AFL style branch coverage shim
///
/// cur_location = <INSTRUMENT_TIME_RANDOM>;
/// shared_mem[cur_location ^ prev_location]++;
/// prev_location = cur_location >> 1;
///
pub fn instrument_with_afl_branch_coverage(m: &mut Module) {
    /*
     * Insert shared memory ptr global varibale
     */
    let trace_bits_ptr = m.add_global(
        ValType::I32,
        Mutability::Mut,
        // init to 0
        vec![Instr::Const(Val::I32(0)), Instr::End],
    );

    // add function for accessing the trace_bits_ptr
    // the wasmer API currently lacks a good procedure for reading
    // exported globals directly, which is why we resort to this slighly
    // over-engineered approach.
    let get_trace_bits_ptr = m.add_function(
        FunctionType::new(&[], &[ValType::I32]),
        vec![],
        vec![Instr::Global(GlobalOp::Get, trace_bits_ptr), Instr::End],
    );
    m.function_mut(get_trace_bits_ptr).export = vec![String::from("get_trace_bits_ptr")];

    // export the trace_bits_ptr such that the host application (AFL) can access it.
    // disabled since wasmtime doesn't like exported non-functions.
    // m.global_mut(trace_bits_ptr).export = vec![String::from("trace_bits_ptr")];

    /*
     * Insert the malloc call allocating the shared memory chunk.
     * The trace_bits_ptr is updated to point to this array.
     */

    let mut is_calloc = false;
    let alloc_func_idx = if let Some((idx, _)) = find_func_by_name("calloc", m.functions_mut()) {
        is_calloc = true;
        idx
    } else if let Some((idx, _)) = find_func_by_name("dlcalloc", m.functions_mut()) {
        is_calloc = true;
        idx
    } else if let Some((idx, _)) = find_func_by_name("malloc", m.functions_mut()) {
        idx
    } else if let Some((idx, _)) = find_func_by_name("dlmalloc", m.functions_mut()) {
        idx
    } else {
        // TODO Other heuristics for identifying a memory allocation function for the
        // trace_bits array. Identifying by name won't work for stripped binaries.
        panic!("cannot find calloc or malloc function")
    };

    // WASI specifies _start must be present.
    let start_func_idx = if let Some((_, f)) = find_func_by_name("_start", m.functions_mut()) {
        //let start_func_idx = if let Some((_, f)) = find_func_by_name("__wasm_call_ctors", m.functions_mut()) {
        f
    } else if let Some((_, f)) = find_func_by_name("main", m.functions_mut()) {
        f
    } else {
        panic!("cannot instrument binary without an exported _start or main function");
    };

    let code = start_func_idx.code_mut().expect("fixme");
    // calloc has a different interface (arguments) than malloc.
    if is_calloc {
        // insert right after the call to __wasm_call_ctors
        code.body.splice(
            1..1,
            vec![
                // the interface calloc(num_items, size)
                Instr::Const(Val::I32(1)),
                Instr::Const(Val::I32(MAP_SIZE as i32)),
                Instr::Call(alloc_func_idx),
                Instr::Global(GlobalOp::Set, trace_bits_ptr),
            ],
        );
    } else {
        // insert right after the call to __wasm_call_ctors
        code.body.splice(
            1..1,
            vec![
                Instr::Const(Val::I32(MAP_SIZE as i32)),
                Instr::Call(alloc_func_idx),
                Instr::Global(GlobalOp::Set, trace_bits_ptr),
            ],
        );
    }

    /*
     * Instrument each function
     */

    // create the prev_location
    let prev_location = m.add_global(
        ValType::I32,
        Mutability::Mut,
        vec![Instr::Const(Val::I32(0)), Instr::End],
    );
    let mut shim_creator = ShimCreator::new(prev_location, trace_bits_ptr);

    // Statistics kept during instrumentation.
    let mut br_if_fallthrough_shims = 0;
    let mut br_table_shims = 0;
    let mut loop_shims = 0;
    let mut if_else_shims = 0;
    let mut function_shims = 0;
    let mut block_end_shims = 0;

    // ignore functions that may be called before the trace_bits array has been initialized.
    let ignored_functions = vec![
        "_start",
        "__wasm_call_ctors",
        "__wasilibc_populate_preopens",
        "__wasi_fd_prestat_get",
        "malloc",
        "__wasi_fd_prestat_dir_name",
        "internal_register_preopened_fd",
        "strdup",
        "strlen",
        "calloc",
        "memcpy",
        "free",
        "dlfree",
        "_Exit",
        "dlmalloc",
        "sbrk",
        "memset",
    ];

    for (_f_idx, f) in m.functions_mut() {
        if let ImportOrPresent::Present(_) = f.code {
            if !f.name.is_some()
                || (!ignored_functions
                    .iter()
                    .any(|&ign_f| f.name.as_ref().unwrap() == ign_f))
            {
                let instrs = &mut f.code_mut().unwrap().body;

                // find the index of instrs for which a shim should follow
                let mut shim_idxs = vec![];
                let mut current_depth: i32 = 0; // i32 since current_depth may become -1 for last end
                                                // TODO change to HashSet<i32>
                let mut jmp_to_depths: Vec<i32> = vec![];
                for (idx, instr) in instrs.iter_mut().enumerate() {
                    match instr {
                        Instr::BrIf(Label(lvl)) => {
                            shim_idxs.push(idx);
                            br_if_fallthrough_shims += 1;

                            jmp_to_depths.push(current_depth - (*lvl as i32));
                        }
                        Instr::If(_) => {
                            current_depth += 1;
                            shim_idxs.push(idx);
                            if_else_shims += 1;
                        }
                        Instr::Else => {
                            shim_idxs.push(idx);
                            if_else_shims += 1;
                        }
                        Instr::Loop(_) => {
                            current_depth += 1;
                            shim_idxs.push(idx);
                            loop_shims += 1;
                        }

                        Instr::BrTable {
                            table,
                            default: Label(default_lvl),
                        } => {
                            // No shim necessary, because execution always branches to a different block.
                            // While technically allowed, no useful instructions follow br_table.

                            // for every level which the br_table can jump to
                            for Label(lvl) in table {
                                jmp_to_depths.push(current_depth - (*lvl as i32));
                            }
                            // also push the default level
                            jmp_to_depths.push(current_depth - (*default_lvl as i32));
                            br_table_shims += 1;
                        }
                        Instr::Block(_) => {
                            current_depth += 1;
                        }
                        Instr::End => {
                            // 0 => end-of-function
                            if current_depth != 0 {
                                // TODO HashSet::take(current_depth) -> Option
                                let jmp_to_depth =
                                    jmp_to_depths.iter().position(|&d| d == current_depth);
                                match jmp_to_depth {
                                    // there is a jump to this end
                                    Some(jmp_to_depth_idx) => {
                                        shim_idxs.push(idx);
                                        block_end_shims += 1;
                                        // remove the end block from jmp_to_depths
                                        // this is crucial since the depth
                                        // might increase again within the
                                        // same nested list of blocks, and a
                                        // shim therefore might be inserted for a
                                        // block which is not targeted by a jump.

                                        // TODO not necessary if jmp_to_depths is a HashSet
                                        jmp_to_depths.remove(jmp_to_depth_idx);
                                    }
                                    // there is no jump to this end
                                    None => (),
                                }
                            }
                            current_depth -= 1;
                        }
                        _ => (),
                    };
                }

                // the shim needs a temporary local storage, create one per function.
                let trace_bits_idx_local = f.add_fresh_local(ValType::I32);
                // insert shims in reverse order to avoid invalidating the indices

                for idx in shim_idxs.iter().rev() {
                    f.code_mut().unwrap().body.splice(
                        idx + 1..idx + 1,
                        shim_creator.get_shim(trace_bits_idx_local),
                    );
                }
                // insert shim at function entrance
                f.code_mut()
                    .unwrap()
                    .body
                    .splice(0..0, shim_creator.get_shim(trace_bits_idx_local));
                function_shims += 1;
            }
        }
    }

    println!("AFL shim insertion summary");
    println!("\tbr_if {}", br_if_fallthrough_shims);
    println!("\tbr_table {}", br_table_shims);
    println!("\tloop {}", loop_shims);
    println!("\tif-else {}", if_else_shims);
    println!("\tfunction enter {}", function_shims);
    println!("\tblock-end {}", block_end_shims);
}

struct ShimCreator {
    rng: ThreadRng,
    //rng: StdRng,
    distribution: Uniform<u32>,
    prev_location: Idx<Global>,
    trace_bits_ptr: Idx<Global>,
}

impl ShimCreator {
    fn new(prev_location: Idx<Global>, trace_bits_ptr: Idx<Global>) -> Self {
        let rng = rand::thread_rng();
        //let rng = StdRng::seed_from_u64(1542);
        let distribution = Uniform::from(0..MAP_SIZE);
        ShimCreator {
            rng,
            distribution,
            prev_location,
            trace_bits_ptr,
        }
    }

    /// returns the AFL shim in webassembly.
    ///
    /// cur_location = <INSTRUMENT_TIME_RANDOM>;
    /// shared_mem[cur_location ^ prev_location]++;
    /// prev_location = cur_location >> 1;
    fn get_shim(&mut self, trace_bits_idx_local: Idx<Local>) -> Vec<Instr> {
        // TODO ensure that value is not used twice until all values have been exhausted
        let cur_location = self.distribution.sample(&mut self.rng);

        // ok to cast to i32 as long as the value is 0...2^16
        vec![
            // compute and store address in the trace_bits_idx_local
            Instr::Const(Val::I32(cur_location as i32)),
            Instr::Global(GlobalOp::Get, self.prev_location),
            Instr::Numeric(NumericOp::I32Xor),
            Instr::Global(GlobalOp::Get, self.trace_bits_ptr),
            Instr::Numeric(NumericOp::I32Add),
            Instr::Local(LocalOp::Tee, trace_bits_idx_local),
            // prepare the memory address for the store
            // TODO. There is no dup instruction, which is why we store
            // the address in a local variable instead.
            // Is there some other more efficient way?
            Instr::Local(LocalOp::Get, trace_bits_idx_local),
            // read, increment and store the prev value
            Instr::Load(
                LoadOp::I32Load8U,
                Memarg {
                    alignment_exp: 0,
                    offset: 0,
                },
            ),
            Instr::Const(Val::I32(1)),
            Instr::Numeric(NumericOp::I32Add),
            Instr::Store(
                StoreOp::I32Store8,
                Memarg {
                    alignment_exp: 0,
                    offset: 0,
                },
            ),
            // set prev_location
            Instr::Const(Val::I32((cur_location >> 1) as i32)),
            Instr::Global(GlobalOp::Set, self.prev_location),
        ]
    }
}

pub fn instrument_file(input_file: &OsStr, output_file: &OsStr) -> Result<(), Box<dyn Error>> {
    let mut module = Module::from_file(&input_file)?;

    self::instrument_with_afl_branch_coverage(&mut module);
    println!("trace_bits size {}", MAP_SIZE);

    module.to_file(output_file)?;
    Ok(())
}
