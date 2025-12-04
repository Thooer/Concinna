#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Bit.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
#[path = "../Impl/Generic/Bit.rs"]
mod backend;

pub fn is_pow2(v: usize) -> bool { backend::impl_is_pow2(v) }
pub fn next_pow2(v: usize) -> usize { backend::impl_next_pow2(v) }
pub fn mask_pow2(v: usize) -> usize { backend::impl_mask_pow2(v) }
pub fn align_up(v: usize, a: usize) -> usize { backend::impl_align_up(v, a) }
pub fn align_down(v: usize, a: usize) -> usize { backend::impl_align_down(v, a) }
