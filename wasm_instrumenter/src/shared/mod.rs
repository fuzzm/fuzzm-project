use wasm::highlevel::{Function, Instr, Local};
use wasm::Idx;

pub fn find_func_by_name<'a>(
    name: &str,
    mut funcs: impl Iterator<Item = (Idx<Function>, &'a mut Function)>,
) -> Option<(Idx<Function>, &'a mut Function)> {
    funcs.find(|(_idx, f)| match f.name.as_ref() {
        Some(n) => n == name,
        None => false,
    })
}

pub fn insert_prefix(f: &mut Function, mut instrs: Vec<Instr>) {
    let code = f
        .code_mut()
        .expect("Cannot insert prefix into function without instructions");

    instrs.append(&mut code.body);
    code.body = instrs;
}

pub fn insert_postfix(f: &mut Function, instrs: &mut Vec<Instr>) {
    let code = f
        .code_mut()
        .expect("Cannot insert postfix into function without instructions");

    let l = code.body.len();
    code.body.splice(l - 1..l - 1, instrs.iter().cloned());
    //code.body.append(&mut vec![Instr::End]);
}

pub fn get_param_n_idx(f: &Function, n: usize) -> Idx<Local> {
    f.params()
        .nth(n)
        .expect("Expected malloc to have at least 1 parameter")
        .0
}
