use cap_log::*;
use cap_path::*;
use cap_stream::*;
use prm_time::*;

fn main() {
    let mut tmp = [0u8; 256];
    let n1 = join("./", "cap_log_smoke.txt", &mut tmp).unwrap();
    let mut path_buf = [0u8; 256];
    let n2 = to_windows(core::str::from_utf8(&tmp[..n1]).unwrap(), &mut path_buf).unwrap();
    let path = core::str::from_utf8(&path_buf[..n2]).unwrap();

    let mut logger = Logger::create(path, LogLevel::Debug).unwrap();
    let t0 = now();
    let _ = logger.log(LogLevel::Info, "hello log");
    let _ = logger.log(LogLevel::Debug, "debug details");
    let _ = logger.flush();
    let t1 = now();
    let dt_ms = to_milliseconds(delta(t0, t1));
    println!("log smoke ok in {} ms", dt_ms);

    println!("log file at {}", path);
}
