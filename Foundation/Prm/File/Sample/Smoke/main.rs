use prm_file::*;

fn main() {
    let path = "file_smoke.bin";
    let h = open(path, FileOpenMode::Create, FileShareMode::ReadWrite).unwrap();
    let w = write(h, b"hello").unwrap();
    assert!(w == 5);
    flush(h).unwrap();
    seek(h, 0, SeekOrigin::Begin).unwrap();
    let mut buf = [0u8; 8];
    let r = read(h, &mut buf).unwrap();
    println!("read {}: {}", r, std::str::from_utf8(&buf[..r]).unwrap());
    let m = map(h, 0, 5, MapAccess::Read).unwrap();
    let s = unsafe { std::slice::from_raw_parts(m.address as *const u8, m.length) };
    println!("map: {}", std::str::from_utf8(s).unwrap());
    unmap(&m).unwrap();
    close(h).unwrap();
}

