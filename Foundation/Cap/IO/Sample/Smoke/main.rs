use cap_io::*;
use prm_file::*;
use prm_time::*;
use cap_path::*;

fn main() {
    let mut tmp = [0u8; 256];
    let n1 = join("./", "cap_io_smoke.bin", &mut tmp).unwrap();
    let mut path_buf = [0u8; 256];
    let n2 = to_windows(core::str::from_utf8(&tmp[..n1]).unwrap(), &mut path_buf).unwrap();
    let path = core::str::from_utf8(&path_buf[..n2]).unwrap();

    let iocp = attach().unwrap();
    let h = open_overlapped(path, FileOpenMode::Create, FileShareMode::ReadWrite).unwrap();
    let _ok = register_handle(&iocp, h.0);

    // write some data synchronously
    let _ = write(h, b"hello async");
    let _ = flush(h);

    // read overlapped
    let mut buf = [0u8; 64];
    let mut ov = Overlapped { internal: 0, internal_high: 0, offset: 0, offset_high: 0, h_event: core::ptr::null_mut() };
    let submitted = read_overlapped(h, &mut buf, 0, &mut ov);
    let got = match get_queued(&iocp, 10) { Some((_ov_ptr, bytes, _key)) => bytes > 0, None => false };
    println!("async read submitted {} got {}", submitted, got);
}
