pub fn md5_digest(data: &[u8]) -> [u8; 16] {
    let mut a: u32 = 0x67452301;
    let mut b: u32 = 0xefcdab89;
    let mut c: u32 = 0x98badcfe;
    let mut d: u32 = 0x10325476;
    let mut msg = Vec::<u8>::with_capacity(((data.len() + 9 + 63) / 64) * 64);
    for &x in data.iter() { msg.push(x); }
    msg.push(0x80);
    while (msg.len() % 64) != 56 { msg.push(0); }
    let bit_len = (data.len() as u64) * 8;
    for i in 0..8 { msg.push(((bit_len >> (8 * i)) & 0xFF) as u8); }
    for chunk in msg.chunks_exact(64) {
        let mut m = [0u32; 16];
        for i in 0..16 {
            let j = i * 4;
            m[i] = (chunk[j] as u32) | ((chunk[j + 1] as u32) << 8) | ((chunk[j + 2] as u32) << 16) | ((chunk[j + 3] as u32) << 24);
        }
        let mut aa = a; let mut bb = b; let mut cc = c; let mut dd = d;
        macro_rules! F { ($x:expr,$y:expr,$z:expr) => { ($x & $y) | (!($x) & $z) } }
        macro_rules! G { ($x:expr,$y:expr,$z:expr) => { ($x & $z) | ($y & !($z)) } }
        macro_rules! H { ($x:expr,$y:expr,$z:expr) => { $x ^ $y ^ $z } }
        macro_rules! I { ($x:expr,$y:expr,$z:expr) => { $y ^ ($x | !($z)) } }
        fn rol(x: u32, s: u32) -> u32 { x.rotate_left(s) }
        macro_rules! R { ($f:ident,$a:ident,$b:ident,$c:ident,$d:ident,$m:expr,$s:expr,$t:expr) => {
            $a = $a.wrapping_add($f!($b,$c,$d)).wrapping_add($m).wrapping_add($t);
            $a = rol($a, $s);
            $a = $a.wrapping_add($b);
        } }
        R!(F, aa, bb, cc, dd, m[0], 7, 0xd76aa478); R!(F, dd, aa, bb, cc, m[1], 12, 0xe8c7b756); R!(F, cc, dd, aa, bb, m[2], 17, 0x242070db); R!(F, bb, cc, dd, aa, m[3], 22, 0xc1bdceee);
        R!(F, aa, bb, cc, dd, m[4], 7, 0xf57c0faf); R!(F, dd, aa, bb, cc, m[5], 12, 0x4787c62a); R!(F, cc, dd, aa, bb, m[6], 17, 0xa8304613); R!(F, bb, cc, dd, aa, m[7], 22, 0xfd469501);
        R!(F, aa, bb, cc, dd, m[8], 7, 0x698098d8); R!(F, dd, aa, bb, cc, m[9], 12, 0x8b44f7af); R!(F, cc, dd, aa, bb, m[10], 17, 0xffff5bb1); R!(F, bb, cc, dd, aa, m[11], 22, 0x895cd7be);
        R!(F, aa, bb, cc, dd, m[12], 7, 0x6b901122); R!(F, dd, aa, bb, cc, m[13], 12, 0xfd987193); R!(F, cc, dd, aa, bb, m[14], 17, 0xa679438e); R!(F, bb, cc, dd, aa, m[15], 22, 0x49b40821);
        R!(G, aa, bb, cc, dd, m[1], 5, 0xf61e2562); R!(G, dd, aa, bb, cc, m[6], 9, 0xc040b340); R!(G, cc, dd, aa, bb, m[11], 14, 0x265e5a51); R!(G, bb, cc, dd, aa, m[0], 20, 0xe9b6c7aa);
        R!(G, aa, bb, cc, dd, m[5], 5, 0xd62f105d); R!(G, dd, aa, bb, cc, m[10], 9, 0x02441453); R!(G, cc, dd, aa, bb, m[15], 14, 0xd8a1e681); R!(G, bb, cc, dd, aa, m[4], 20, 0xe7d3fbc8);
        R!(G, aa, bb, cc, dd, m[9], 5, 0x21e1cde6); R!(G, dd, aa, bb, cc, m[14], 9, 0xc33707d6); R!(G, cc, dd, aa, bb, m[3], 14, 0xf4d50d87); R!(G, bb, cc, dd, aa, m[8], 20, 0x455a14ed);
        R!(G, aa, bb, cc, dd, m[13], 5, 0xa9e3e905); R!(G, dd, aa, bb, cc, m[2], 9, 0xfcefa3f8); R!(G, cc, dd, aa, bb, m[7], 14, 0x676f02d9); R!(G, bb, cc, dd, aa, m[12], 20, 0x8d2a4c8a);
        R!(H, aa, bb, cc, dd, m[5], 4, 0xfffa3942); R!(H, dd, aa, bb, cc, m[8], 11, 0x8771f681); R!(H, cc, dd, aa, bb, m[11], 16, 0x6d9d6122); R!(H, bb, cc, dd, aa, m[14], 23, 0xfde5380c);
        R!(H, aa, bb, cc, dd, m[1], 4, 0xa4beea44); R!(H, dd, aa, bb, cc, m[4], 11, 0x4bdecfa9); R!(H, cc, dd, aa, bb, m[7], 16, 0xf6bb4b60); R!(H, bb, cc, dd, aa, m[10], 23, 0xbebfbc70);
        R!(H, aa, bb, cc, dd, m[13], 4, 0x289b7ec6); R!(H, dd, aa, bb, cc, m[0], 11, 0xeaa127fa); R!(H, cc, dd, aa, bb, m[3], 16, 0xd4ef3085); R!(H, bb, cc, dd, aa, m[6], 23, 0x04881d05);
        R!(I, aa, bb, cc, dd, m[0], 6, 0xf4292244); R!(I, dd, aa, bb, cc, m[7], 10, 0x432aff97); R!(I, cc, dd, aa, bb, m[14], 15, 0xab9423a7); R!(I, bb, cc, dd, aa, m[5], 21, 0xfc93a039);
        R!(I, aa, bb, cc, dd, m[12], 6, 0x655b59c3); R!(I, dd, aa, bb, cc, m[3], 10, 0x8f0ccc92); R!(I, cc, dd, aa, bb, m[10], 15, 0xffeff47d); R!(I, bb, cc, dd, aa, m[1], 21, 0x85845dd1);
        R!(I, aa, bb, cc, dd, m[8], 6, 0x6fa87e4f); R!(I, dd, aa, bb, cc, m[15], 10, 0xfe2ce6e0); R!(I, cc, dd, aa, bb, m[6], 15, 0xa3014314); R!(I, bb, cc, dd, aa, m[13], 21, 0x4e0811a1);
        a = a.wrapping_add(aa); b = b.wrapping_add(bb); c = c.wrapping_add(cc); d = d.wrapping_add(dd);
    }
    let mut out = [0u8; 16];
    for (i, v) in [a, b, c, d].iter().enumerate() {
        let j = i * 4; let x = *v;
        out[j] = (x & 0xFF) as u8; out[j + 1] = ((x >> 8) & 0xFF) as u8; out[j + 2] = ((x >> 16) & 0xFF) as u8; out[j + 3] = ((x >> 24) & 0xFF) as u8;
    }
    out
}
