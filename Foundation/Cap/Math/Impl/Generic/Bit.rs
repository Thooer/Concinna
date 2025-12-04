pub fn impl_is_pow2(v: usize) -> bool { v != 0 && (v & (v - 1)) == 0 }
pub fn impl_next_pow2(mut v: usize) -> usize { if v <= 1 { return 1; } v -= 1; v |= v >> 1; v |= v >> 2; v |= v >> 4; v |= v >> 8; v |= v >> 16; if core::mem::size_of::<usize>() >= 8 { v |= v >> 32; } v + 1 }
pub fn impl_mask_pow2(v: usize) -> usize { if impl_is_pow2(v) { v - 1 } else { impl_next_pow2(v) - 1 } }
pub fn impl_align_up(v: usize, a: usize) -> usize { ((v + (a - 1)) / a) * a }
pub fn impl_align_down(v: usize, a: usize) -> usize { (v / a) * a }
