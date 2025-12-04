#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Random { pub state: u64 }

impl Random {
    pub fn with_seed(seed: u64) -> Self { let mut s = seed; if s == 0 { s = 0x9e3779b97f4a7c15; } Self { state: s } }
    fn next64(&mut self) -> u64 { let mut x = self.state; x ^= x << 13; x ^= x >> 7; x ^= x << 17; self.state = x; x }
    pub fn next_u32(&mut self) -> u32 { (self.next64() >> 32) as u32 }
    pub fn next_u64(&mut self) -> u64 { self.next64() }
    pub fn next_f32(&mut self) -> f32 { let x = self.next_u32(); (x as f32) / (u32::MAX as f32) }
    pub fn uniform_u32(&mut self, max_exclusive: u32) -> u32 { if max_exclusive == 0 { 0 } else { self.next_u32() % max_exclusive } }
    pub fn uniform_f32(&mut self, min: f32, max: f32) -> f32 { let r = self.next_f32(); min + (max - min) * r }
    pub fn normal_f32(&mut self, mean: f32, stddev: f32) -> f32 {
        let u1 = self.next_f32().max(1e-12);
        let u2 = self.next_f32();
        let z0 = ( -2.0 * u1.ln() ).sqrt() * (2.0 * core::f32::consts::PI * u2).cos();
        mean + z0 * stddev
    }
}

