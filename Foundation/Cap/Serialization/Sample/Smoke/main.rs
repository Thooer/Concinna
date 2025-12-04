use cap_serialization::*;
use cap_stream::*;
use cap_path::*;

fn main() {
    let mut tmp = [0u8; 256];
    let n1 = join("./", "cap_serialization_smoke.bin", &mut tmp).unwrap();
    let mut path_buf = [0u8; 256];
    let n2 = to_windows(core::str::from_utf8(&tmp[..n1]).unwrap(), &mut path_buf).unwrap();
    let path = core::str::from_utf8(&path_buf[..n2]).unwrap();

    let mut w = FileWriter::create(path).unwrap();
    let _ = write_u32(&mut w, 123456789);
    let _ = write_f32(&mut w, 3.14159);
    let _ = write_bytes(&mut w, b"hi");
    let _ = w.flush();
    drop(w);
    let mut r = FileReader::open(path).unwrap();
    let a = read_u32(&mut r).unwrap();
    let b = read_f32(&mut r).unwrap();
    let mut buf = [0u8; 8];
    let s = read_exact(&mut r, &mut buf).unwrap();
    println!("serial ok {} {} {}", a, b, core::str::from_utf8(s).unwrap());
}
