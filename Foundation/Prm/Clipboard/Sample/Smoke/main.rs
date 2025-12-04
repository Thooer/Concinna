use prm_clipboard::*;

fn main() {
    let s = "hello剪贴板";
    set_text(s).unwrap();
    let mut buf = [0u8; 64];
    let n = get_text(&mut buf).unwrap();
    let got = std::str::from_utf8(&buf[..n]).unwrap();
    assert_eq!(got, s);
    println!("Clipboard OK");
}

