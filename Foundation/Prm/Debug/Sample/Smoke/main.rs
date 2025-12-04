use prm_debug::*;

fn main() {
    output_debug_string("Prm.Debug Smoke");
    let mut buf = [0u8; 256];
    let n = format_error_message(2, &mut buf).unwrap();
    let msg = std::str::from_utf8(&buf[..n]).unwrap();
    println!("err2={}", msg.trim());
    let mut frames = [0usize; 16];
    let c = capture_backtrace(0, &mut frames);
    assert!(c > 0);
    println!("frames={}", c);
    symbols_init().unwrap();
    let mut resolved = false;
    for &addr in &frames {
        if addr == 0 { continue; }
        let mut name = [0u8; 256];
        if let Ok(s) = resolve_symbol(addr, &mut name) {
            let symbol = std::str::from_utf8(&name[..s]).unwrap();
            let mut file = [0u8; 260];
            if let Ok((f, line)) = resolve_line(addr, &mut file) {
                let file_path = std::str::from_utf8(&file[..f]).unwrap();
                println!("symbol={} at {}:{}", symbol, file_path, line);
            } else {
                println!("symbol={}", symbol);
            }
            resolved = true;
            break;
        }
    }
    if !resolved { println!("no symbol resolved"); }
    symbols_cleanup();
    println!("Debugger={} LastError={}", is_debugger_present(), get_last_error());
}
