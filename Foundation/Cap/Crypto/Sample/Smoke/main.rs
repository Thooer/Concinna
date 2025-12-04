use cap_crypto::*;

fn hex(b: &[u8]) -> String {
    const HEX: &[u8] = b"0123456789abcdef";
    let mut out = String::with_capacity(b.len()*2);
    for &x in b { out.push(HEX[(x>>4) as usize] as char); out.push(HEX[(x&0xF) as usize] as char); }
    out
}

fn main() {
    let md5 = md5_digest(b"abc");
    let sha = sha256_digest(b"abc");
    println!("md5 {}", hex(&md5));
    println!("sha256 {}", hex(&sha));
}

