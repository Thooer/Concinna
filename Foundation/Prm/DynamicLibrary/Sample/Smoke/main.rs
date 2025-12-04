use prm_dynamiclibrary::*;

fn main() {
    let lib = load("kernel32.dll").unwrap();
    let sym = get_symbol(lib, "GetTickCount").unwrap();
    assert!(!sym.is_null());
    close(lib);
    println!("DynamicLibrary OK");
}

