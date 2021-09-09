use crate::shared::{find_func_by_name, get_param_n_idx, insert_postfix, insert_prefix};
use std::error::Error;
use std::ffi::OsStr;
use wasm::highlevel::{
    Function, Global, GlobalOp, Instr, LoadOp, Local, LocalOp, Module, NumericOp, StoreOp,
};
use rand::distributions::{Distribution, Uniform};
use wasm::{BlockType, FunctionType, Idx, Label, Memarg, Mutability, Val, ValType};

pub fn instrument_file(
    input_file: &OsStr,
    output_file: &OsStr,
    skip_heap: bool,
    skip_stack: bool,
    skip_print: bool,
) -> Result<(), Box<dyn Error>> {
    let mut module = Module::from_file(&input_file)?;

    if !skip_heap {
        self::instrument_with_heap_canary_check(&mut module);
    }

    if !skip_stack {
        self::instrument_with_stack_canary_check(&mut module);
    }

    if !skip_print {
        self::instrument_with_print_functions(&mut module);
    }

    module.to_file(output_file)?;
    Ok(())
}

/// Inserts heap canaries and checks.
/// A newly allocated heap block is initialized as follows:
/// | -- size (4 bytes) -- | -- underflow canary (8 bytes) -- | -- user data (n bytes) -- | -- overflow canary (8 bytes ) -- |
/// The size includes the size field itself and the two canaries (size = 4 + 8 + n + 8).
/// Each canary is initialized to all zeros.
/// TODO use static random value for canary instead to also detect zero-overflows.
/// The canaries are checked at calls to free and realloc.

pub fn instrument_with_heap_canary_check(m: &mut Module) {
    // We disable canary checks in calls to free that occur
    // within the realloc function since the block passed to free does not have canary values.
    // Notice, realloc itself performs canary checking at its entry, so this does not
    // result in missed buffer overflows.
    let disable_canary_glob = m.add_global(
        ValType::I32,
        Mutability::Mut,
        vec![Instr::Const(Val::I32(0)), Instr::End],
    );

    // We disable canary insertion when malloc is called within calloc.
    // calloc itself will insert the canaries at its exit.
    // calloc requires access to metadata of the heap chunk returned by malloc, which is why this
    // is required.
    let disable_canary_insertion_in_malloc = m.add_global(
        ValType::I32,
        Mutability::Mut,
        vec![Instr::Const(Val::I32(0)), Instr::End],
    );

    // TODO detect malloc by other heuristics than by name.
    let (_, malloc) = find_func_by_name("dlmalloc", m.functions_mut())
        .expect("Aborting: Could not find malloc function");

    let malloc_p0 = get_param_n_idx(malloc, 0);
    let malloc_size = malloc.add_fresh_local(ValType::I32);

    insert_prefix(
        malloc,
        // preamble for malloc (reserve an extra 20 bytes)
        // 2 canaries take 16 bytes and the size takes 4 bytes.
        // Notice, we do not reuse the size field from the chunk since
        // 1. it may contain flag bits which we have to account for
        // 2. the returned chunk may the larger than the requested size
        // resulting in canaries being placed unnecessarily far apart.
        vec![
            // check if canary insertion is disabled
            Instr::Block(BlockType(None)),
            Instr::Global(GlobalOp::Get, disable_canary_insertion_in_malloc),
            Instr::Const(Val::I32(1)),
            Instr::Numeric(NumericOp::I32Eq),
            Instr::BrIf(Label(0)),
            Instr::Local(LocalOp::Get, malloc_p0),
            //TODO do we also need to 16 byte align the heap values?
            //I don't think that is necessary, but it might be worth while to check.
            Instr::Const(Val::I32(20)),
            Instr::Numeric(NumericOp::I32Add),
            // store the size in a fresh local in case the first parameter gets overwritten
            Instr::Local(LocalOp::Tee, malloc_size),
            Instr::Local(LocalOp::Set, malloc_p0),
            Instr::End,
        ],
    );

    // wrap the postfix in a function since we need to use it for both malloc
    // and for realloc, but we need different locals to store the tmp result and size.
    let get_malloc_postfix = |func: &mut Function,
                              size_local: Idx<Local>,
                              disable_check: Option<Vec<Instr>>|
     -> Vec<Instr> {
        let tmp_result_local = func.add_fresh_local(ValType::I32);
        let mut postfix = vec![
            Instr::Local(LocalOp::Set, tmp_result_local),
            Instr::Block(BlockType(Some(ValType::I32))),
            // if result is NULL, then don't attempt to insert canaries.
            Instr::Local(LocalOp::Get, tmp_result_local),
            Instr::Local(LocalOp::Get, tmp_result_local),
            Instr::Numeric(NumericOp::I32Eqz),
            Instr::BrIf(Label(0)),
        ];
        if disable_check.is_some() {
            postfix.append(&mut disable_check.unwrap());
            //postfix.append(&mut );
        }
        // store malloc result (ptr to data) in a new local
        // store the size at the first 4 bytes
        postfix.append(&mut vec![
            Instr::Local(LocalOp::Get, size_local),
            Instr::Store(
                StoreOp::I32Store,
                Memarg {
                    alignment_exp: 0,
                    offset: 0,
                },
            ),
            // store the first canary at byte 4 to 11
            Instr::Local(LocalOp::Get, tmp_result_local),
            Instr::Const(Val::I64(0)),
            Instr::Store(
                StoreOp::I64Store,
                Memarg {
                    alignment_exp: 0,
                    offset: 4,
                },
            ),
            // get the size
            Instr::Local(LocalOp::Get, size_local),
            // add with x to get the index of the last data chunk
            Instr::Local(LocalOp::Get, tmp_result_local),
            Instr::Numeric(NumericOp::I32Add),
            // index (ptr + size - 8) where we want to place the canary
            Instr::Const(Val::I32(8)),
            Instr::Numeric(NumericOp::I32Sub),
            // store post data canary
            Instr::Const(Val::I64(0)),
            Instr::Store(
                StoreOp::I64Store,
                Memarg {
                    alignment_exp: 0,
                    offset: 0,
                },
            ),
            // return ptr + 12, i.e., ptr to first element after the first canary.
            Instr::Local(LocalOp::Get, tmp_result_local),
            Instr::Const(Val::I32(12)),
            Instr::Numeric(NumericOp::I32Add),
            Instr::End,
        ]);
        postfix
    };
    let mut post_fix = get_malloc_postfix(
        malloc,
        malloc_size,
        Some(vec![
            // check if canary insertion is disabled
            Instr::Global(GlobalOp::Get, disable_canary_insertion_in_malloc),
            Instr::Const(Val::I32(1)),
            Instr::Numeric(NumericOp::I32Eq),
            // returns tmp_result_local pushed in malloc postfix
            Instr::BrIf(Label(0)),
        ]),
    );
    insert_postfix(malloc, &mut post_fix);

    let (_, free) = find_func_by_name("dlfree", m.functions_mut())
        .expect("Aborting: Could not find free function");

    fn get_canary_check_block(
        chunk_data_ptr: Idx<Local>,
        disable_canary_glob: Idx<Global>,
    ) -> Vec<Instr> {
        vec![
            // if disable_canary_glob then skip the checks
            Instr::Block(BlockType(None)),
            Instr::Global(GlobalOp::Get, disable_canary_glob),
            Instr::Const(Val::I32(1)),
            Instr::Numeric(NumericOp::I32Eq),
            Instr::BrIf(Label(0)),
            // if ptr == NULL, then skip the canary checks
            Instr::Block(BlockType(None)),
            Instr::Local(LocalOp::Get, chunk_data_ptr),
            Instr::Numeric(NumericOp::I32Eqz),
            Instr::BrIf(Label(0)),
            // block to check first canary
            Instr::Block(BlockType(None)),
            // decrease ptr by 12
            Instr::Local(LocalOp::Get, chunk_data_ptr),
            Instr::Const(Val::I32(12)),
            Instr::Numeric(NumericOp::I32Sub),
            Instr::Local(LocalOp::Set, chunk_data_ptr),
            // retrieve ptr to first canary (param_0 - 8)
            Instr::Local(LocalOp::Get, chunk_data_ptr),
            Instr::Const(Val::I32(4)),
            Instr::Numeric(NumericOp::I32Add),
            // check first parameter
            Instr::Load(
                LoadOp::I64Load,
                Memarg {
                    alignment_exp: 0,
                    offset: 0,
                },
            ),
            Instr::Numeric(NumericOp::I64Eqz),
            Instr::BrIf(Label(0)),
            Instr::Unreachable,
            Instr::End,
            // block to check the second canary
            Instr::Block(BlockType(None)),
            // fetch the size of the chunk
            Instr::Local(LocalOp::Get, chunk_data_ptr),
            Instr::Load(
                LoadOp::I32Load,
                Memarg {
                    alignment_exp: 0,
                    offset: 0,
                },
            ),
            // get the index of the last data item + 12 bytes (since param 0 is real data ptr + 12)
            Instr::Local(LocalOp::Get, chunk_data_ptr),
            Instr::Numeric(NumericOp::I32Add),
            // get the index of the post-chunk canary
            Instr::Const(Val::I32(8)),
            Instr::Numeric(NumericOp::I32Sub),
            // check the canary value
            Instr::Load(
                LoadOp::I64Load,
                Memarg {
                    alignment_exp: 0,
                    offset: 0,
                },
            ),
            Instr::Numeric(NumericOp::I64Eqz),
            Instr::BrIf(Label(0)),
            Instr::Unreachable,
            Instr::End,
            Instr::End,
            Instr::End,
        ]
    }

    let free_p0 = get_param_n_idx(free, 0);
    let free_prefix = get_canary_check_block(free_p0, disable_canary_glob);
    insert_prefix(free, free_prefix);

    let mut realloc = find_func_by_name("dlrealloc", m.functions_mut());
    realloc = match realloc {
        None => find_func_by_name("realloc", &mut m.functions_mut()),
        x => x,
    };
    match realloc {
        Some((_, realloc)) => {
            // the ptr
            let realloc_p0 = get_param_n_idx(realloc, 0);
            // the size
            let realloc_p1 = get_param_n_idx(realloc, 1);
            let realloc_size = realloc.add_fresh_local(ValType::I32);

            returns_to_outer_block_jmp(realloc);

            let mut realloc_prefix = get_canary_check_block(realloc_p0, disable_canary_glob);

            realloc_prefix.append(&mut vec![
                // reserve another 20 bytes for the two canaries and the size
                Instr::Local(LocalOp::Get, realloc_p1),
                Instr::Const(Val::I32(20)),
                Instr::Numeric(NumericOp::I32Add),
                Instr::Local(LocalOp::Tee, realloc_size),
                Instr::Local(LocalOp::Set, realloc_p1),
                // disable canary checking while the body of realloc is executing
                Instr::Const(Val::I32(1)),
                Instr::Global(GlobalOp::Set, disable_canary_glob),
            ]);
            insert_prefix(realloc, realloc_prefix);

            // reuse the malloc postfix
            let mut post_fix = get_malloc_postfix(realloc, realloc_size, None);
            // re-enable the canary checking
            post_fix.push(Instr::Const(Val::I32(0)));
            post_fix.push(Instr::Global(GlobalOp::Set, disable_canary_glob));
            insert_postfix(realloc, &mut post_fix);
        }
        None => println!("realloc not detected. Skipping instrumentation"),
    };

    let mut calloc = find_func_by_name("dlcalloc", m.functions_mut());
    calloc = match calloc {
        None => find_func_by_name("calloc", &mut m.functions_mut()),
        x => x,
    };

    match calloc {
        Some((_, calloc)) => {
            // the ptr
            let calloc_p0 = get_param_n_idx(calloc, 0);
            // the size
            let calloc_p1 = get_param_n_idx(calloc, 1);
            let calloc_size = calloc.add_fresh_local(ValType::I32);

            // set to 1 if calloc overflows
            let calloc_overflow = calloc.add_fresh_local(ValType::I32);

            returns_to_outer_block_jmp(calloc);

            // calloc (nitems, item_size)
            let calloc_prefix = vec![
                // disable canary insertion in malloc
                Instr::Const(Val::I32(1)),
                Instr::Global(GlobalOp::Set, disable_canary_insertion_in_malloc),
                // check if nitems * item_size overflows.
                // In that case, calloc should fail, so it's essential we don't modify the args.
                Instr::Local(LocalOp::Get, calloc_p0),
                Instr::Numeric(NumericOp::I64ExtendI32U),
                Instr::Local(LocalOp::Get, calloc_p1),
                Instr::Numeric(NumericOp::I64ExtendI32U),
                Instr::Numeric(NumericOp::I64Mul),
                Instr::Const(Val::I64(0xFFFFFFFF)),
                Instr::Numeric(NumericOp::I64GeU),
                Instr::If(BlockType(None)),
                Instr::Const(Val::I32(1)),
                Instr::Local(LocalOp::Set, calloc_overflow),
                Instr::Else,
                Instr::Const(Val::I32(0)),
                Instr::Local(LocalOp::Set, calloc_overflow),
                // calloc (nitems, item_size) = calloc (1, item_size * nitems)
                Instr::Local(LocalOp::Get, calloc_p0),
                Instr::Local(LocalOp::Get, calloc_p1),
                Instr::Numeric(NumericOp::I32Mul),
                Instr::Local(LocalOp::Set, calloc_p1),
                Instr::Const(Val::I32(1)),
                Instr::Local(LocalOp::Set, calloc_p0),
                // reserve another 20 bytes for canaries and size
                Instr::Local(LocalOp::Get, calloc_p1),
                Instr::Const(Val::I32(20)),
                Instr::Numeric(NumericOp::I32Add),
                Instr::Local(LocalOp::Tee, calloc_size),
                Instr::Local(LocalOp::Set, calloc_p1),
                Instr::End,
            ];

            insert_prefix(calloc, calloc_prefix);

            // reuse the malloc postfix
            let mut post_fix = get_malloc_postfix(
                calloc,
                calloc_size,
                Some(vec![
                    Instr::Local(LocalOp::Get, calloc_overflow),
                    // check for overflow
                    Instr::Const(Val::I32(1)),
                    Instr::Numeric(NumericOp::I32Eq),
                    // returns tmp_result_local pushed in malloc postfix
                    Instr::BrIf(Label(0)),
                ]),
            );
            // re-enable the canary insertion in malloc
            post_fix.push(Instr::Const(Val::I32(0)));
            post_fix.push(Instr::Global(
                GlobalOp::Set,
                disable_canary_insertion_in_malloc,
            ));
            insert_postfix(calloc, &mut post_fix);
        }
        None => println!("calloc not detected. Skipping instrumentation"),
    };
}

static CANARY_ALLOC_BLOCK_SIZE: i32 = 16;
static CANARY_SIZE_BYTES: u32 = 8;

/**
 * Add a stack canary check to functions.
 * An 8-byte all-zero value is pushed to the stack at function entry.
 * If, at function exit, this value has changed, then the program will terminate.
 *
 * The instrumentation wraps the function body in a block.
 * Every return in the block is changed to a break that exits the block.
 * Instructions for inserting the canary are inserted before the block,
 * and instructions for checking the canary are inserted after.
 */
pub fn instrument_with_stack_canary_check(m: &mut Module) {
    let stack_ptr_idx = find_stack_ptr(m);
    //let rng = StdRng::seed_from_u64(1542);
    let mut rng = rand::thread_rng();
    let base: i64 = 2;
    let distribution = Uniform::from(0..(base.pow((CANARY_SIZE_BYTES * 8) - 2)));

    m.functions_mut().for_each(|(_, f)| {
        if requires_canary_check(f, &stack_ptr_idx) {
            let canary_value = distribution.sample(&mut rng);
           
            if f.code().is_some() {
                returns_to_outer_block_jmp(f);

                // create canary insertion instructions
                insert_prefix(
                    f,
                    vec![
                        Instr::Global(GlobalOp::Get, stack_ptr_idx),
                        Instr::Const(Val::I32(CANARY_ALLOC_BLOCK_SIZE)),
                        Instr::Numeric(NumericOp::I32Sub),
                        Instr::Global(GlobalOp::Set, stack_ptr_idx),
                        Instr::Global(GlobalOp::Get, stack_ptr_idx),
                        Instr::Const(Val::I64(canary_value)),
                        Instr::Store(
                            StoreOp::I64Store,
                            Memarg {
                                alignment_exp: 0,
                                offset: 0,
                            },
                        ),
                    ],
                );

                // if the return type is non-void (tmp_result_local exists)
                // then insert an instruction inserting the block return value
                // into this tmp local.
                let mut post_fix: Vec<Instr> = vec![];

                // create canary check instructions
                post_fix.append(&mut vec![
                    Instr::Block(BlockType(None)),
                    Instr::Global(GlobalOp::Get, stack_ptr_idx),
                    Instr::Load(
                        LoadOp::I64Load,
                        Memarg {
                            alignment_exp: 0,
                            offset: 0,
                        },
                    ),
                    Instr::Const(Val::I64(canary_value)),
                    Instr::Numeric(NumericOp::I64Eq),
                    Instr::BrIf(Label(0)),
                    Instr::Unreachable,
                    Instr::End,
                    Instr::Global(GlobalOp::Get, stack_ptr_idx),
                    Instr::Const(Val::I32(CANARY_ALLOC_BLOCK_SIZE)),
                    Instr::Numeric(NumericOp::I32Add),
                    Instr::Global(GlobalOp::Set, stack_ptr_idx),
                ]);

                insert_postfix(f, &mut post_fix);
            }
        }
    });
}

pub fn instrument_with_print_functions(m: &mut Module) {
    let stack_ptr_idx = find_stack_ptr(m);
    let (printf_idx, _) = find_func_by_name("printf", m.functions_mut())
        .expect("Cannot instrument with print functions in binary without printf");

    // we need malloc to allocate space for the string literal
    let (malloc_idx, _) = find_func_by_name("malloc", m.functions_mut())
        .expect("Cannot instrument with print functions in binary without malloc");

    let mut printf_func = |name: &str, str_lit_bytes: i32| {
        let print32idx = m.add_function(FunctionType::new(&[ValType::I32], &[]), vec![], vec![]);
        let mut print32 = m.function_mut(print32idx);
        print32.name = Some(String::from(name));
        // unwrap safe, since we created function with 1 param
        let (p0, _) = print32.params().next().unwrap();
        let l1 = print32.add_fresh_local(ValType::I32);
        let l2 = print32.add_fresh_local(ValType::I32);
        print32.instrs_mut().unwrap().append(&mut vec![
            Instr::Global(GlobalOp::Get, stack_ptr_idx), //global.get 0
            Instr::Const(Val::I32(16)),                  //i32.const 16
            Instr::Numeric(NumericOp::I32Sub),           //i32.sub
            Instr::Local(LocalOp::Tee, l1),              //local.tee 1
            Instr::Global(GlobalOp::Set, stack_ptr_idx), //global.set 0
            Instr::Const(Val::I32(6)),                   //i32.const 6
            Instr::Call(malloc_idx),                     //call $malloc
            Instr::Local(LocalOp::Tee, l2),              //local.tee 2
            Instr::Const(Val::I32(10)),                  //i32.const 10
            Instr::Store(
                StoreOp::I32Store16,
                Memarg {
                    alignment_exp: 1,
                    offset: 4,
                },
            ), //i32.store16 offset=4 align=1
            Instr::Local(LocalOp::Get, l2),              //local.get 2
            Instr::Const(Val::I32(str_lit_bytes)),       //i32.const string literal bytes
            Instr::Store(
                StoreOp::I32Store,
                Memarg {
                    alignment_exp: 1,
                    offset: 0,
                },
            ), //i32.store align=1
            Instr::Local(LocalOp::Get, l1),              //local.get 1
            Instr::Local(LocalOp::Get, p0),              //local.get 0
            Instr::Store(
                StoreOp::I32Store,
                Memarg {
                    alignment_exp: 0,
                    offset: 0,
                },
            ), //i32.store
            Instr::Local(LocalOp::Get, l2),              //local.get 2
            Instr::Local(LocalOp::Get, l1),              //local.get 1
            Instr::Call(printf_idx),                     //call $printf
            Instr::Drop,                                 //drop
            Instr::Local(LocalOp::Get, l1),              //local.get 1
            Instr::Const(Val::I32(16)),                  //i32.const 16
            Instr::Numeric(NumericOp::I32Add),           //i32.add
            Instr::Global(GlobalOp::Set, stack_ptr_idx), //global.set 0,
            Instr::End,
        ]);
    };

    printf_func("print32", 2016686117);
    printf_func("print64", 2016948261);
}

fn find_stack_ptr(m: &Module) -> Idx<Global> {
    //TODO is the stack pointer always the first global?
    m.globals()
        .nth(0)
        .expect("could not detect stack pointer")
        .0
}

fn requires_canary_check(f: &Function, stack_ptr: &Idx<Global>) -> bool {
    // emscripten adds the stackAlloc and stackRestore functions
    // that both break the invarint that the stack size prior to function
    // entrance is equal to the stack size after function exit.
    // The stackAlloc function is used to, for example, allocate space for argv
    // before the main function is called.
    let breaks_stack_invariant = is_stack_alloc(f, stack_ptr) || is_stack_restore(f, stack_ptr);
    // we check instr_count > 1 since all functions implicitly have an end instruction.
    f.instr_count() > 1 && !breaks_stack_invariant
    // possible other optimizations?
    // TODO check if f even uses the stack pointer.
}

/**
 * Checks if the instructions of f matches the instructions of the emscripten stackAlloc function.
 * This function will break if the instructions of stackAlloc changes.
 */
fn is_stack_alloc(f: &Function, stack_ptr: &Idx<Global>) -> bool {
    let instrs = f.instrs();
    if instrs.len() != 9 {
        return false;
    }

    match (
        &instrs[0], &instrs[1], &instrs[2], &instrs[3], &instrs[4], &instrs[5], &instrs[6],
        &instrs[7], &instrs[8],
    ) {
        (
            Instr::Global(GlobalOp::Get, sp1),
            Instr::Local(LocalOp::Get, _),
            Instr::Numeric(NumericOp::I32Sub),
            Instr::Const(Val::I32(-16)),
            Instr::Numeric(NumericOp::I32And),
            Instr::Local(LocalOp::Tee, l1),
            Instr::Global(GlobalOp::Set, sp2),
            Instr::Local(LocalOp::Get, l1_alt),
            Instr::End,
        ) => sp1 == stack_ptr && sp2 == stack_ptr && l1 == l1_alt,
        _ => false,
    }
}

fn is_stack_restore(f: &Function, stack_ptr: &Idx<Global>) -> bool {
    let instrs = f.instrs();
    if instrs.len() != 3 {
        return false;
    }

    match (&instrs[0], &instrs[1], &instrs[2]) {
        (Instr::Local(LocalOp::Get, _), Instr::Global(GlobalOp::Set, sp), Instr::End) => {
            sp == stack_ptr
        }
        _ => false,
    }
}

/**
 * Returns None for void functions
 */
fn get_return_type(f: &Function) -> Option<ValType> {
    if f.type_.results.len() > 0 {
        // There can always only be 1 return type.
        Some(f.type_.results[0])
    } else {
        None
    }
}

struct ReturnInstrLoc {
    // The index of the return instruction relative to the first instruction
    idx: usize,
    // The depth of the scope containing the return instruction.
    // where 0 is the scope of the function body.
    level: i32,
}

/**
 * wraps instrs in a block and replaces all returns in instrs with a jmp to that block
 */
fn returns_to_outer_block_jmp(f: &mut Function) {
    let return_type = get_return_type(f);
    if let Some(code) = f.code_mut() {
        let instrs = &mut code.body;
        let return_locs = get_return_instr_locs(&instrs);

        for r_loc in return_locs {
            instrs[r_loc.idx] = Instr::Br(Label(r_loc.level as u32));
        }

        // insert the block wrapper around the function body.
        instrs.insert(0, Instr::Block(BlockType(return_type)));
        instrs.push(Instr::End);
    }
}

fn get_return_instr_locs(instrs: &Vec<Instr>) -> Vec<ReturnInstrLoc> {
    let mut level = 0;
    let return_instr_locs = instrs
        .iter()
        .zip(0..instrs.len())
        .fold::<Vec<ReturnInstrLoc>, _>(vec![], |mut acc, (instr, idx)| {
            match instr {
                Instr::Return => acc.push(ReturnInstrLoc { level, idx }),
                Instr::Block(_) | Instr::If(_) | Instr::Loop(_) => level += 1,
                Instr::End => level -= 1,
                _ => (),
            };
            acc
        });
    assert!(
        level == -1,
        "Function body unexpectedly does not end in the function body scope"
    );
    return_instr_locs
}
