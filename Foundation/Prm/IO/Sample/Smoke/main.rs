use prm_io::*;

fn main() {
    let path = "io_smoke_tmp.txt";
    let h = open(path, FileOpenMode::Create, FileShareMode::ReadWrite).unwrap();
    write(h, b"abc").unwrap();
    close(h).unwrap();
    let h2 = open(path, FileOpenMode::Read, FileShareMode::Read).unwrap();
    let mut buf = [0u8; 8];
    let n = read(h2, &mut buf).unwrap();
    assert_eq!(&buf[..n], b"abc");
    close(h2).unwrap();
    path_remove_file(path).unwrap();
    println!("IO OK");
}

