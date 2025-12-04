use cap_random::*;
use cap_crypto::*;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Guid { pub bytes: [u8; 16] }

pub fn uuid_v4(r: &mut Random) -> Guid {
    let mut b = [0u8; 16];
    for i in 0..4 { let x = r.next_u32(); b[i*4] = (x & 0xFF) as u8; b[i*4+1] = ((x >> 8) & 0xFF) as u8; b[i*4+2] = ((x >> 16) & 0xFF) as u8; b[i*4+3] = ((x >> 24) & 0xFF) as u8; }
    b[6] = (b[6] & 0x0F) | 0x40;
    b[8] = (b[8] & 0x3F) | 0x80;
    Guid { bytes: b }
}

pub fn uuid_to_string(g: &Guid) -> String {
    const HEX: &[u8] = b"0123456789abcdef";
    let mut s = [0u8; 36]; let mut i = 0usize; let mut j = 0usize;
    while j < 16 {
        if j == 4 || j == 6 || j == 8 || j == 10 { s[i] = b'-'; i+=1; }
        let b = g.bytes[j]; s[i] = HEX[(b>>4) as usize]; s[i+1] = HEX[(b&0xF) as usize]; i+=2; j+=1;
    }
    String::from_utf8_lossy(&s).to_string()
}

pub fn string_id64(s: &str) -> u64 {
    let d = sha256_digest(s.as_bytes());
    let mut x = 0u64; for i in 0..8 { x = (x << 8) | d[i] as u64; } x
}

