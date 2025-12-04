#[derive(Clone, Copy, Debug, PartialEq)]
pub struct MemoryBlock { pub ptr: *mut u8, pub size: usize }

impl MemoryBlock {
    pub fn new(ptr: *mut u8, size: usize) -> Self { Self { ptr, size } }
    pub fn empty() -> Self { Self { ptr: core::ptr::null_mut(), size: 0 } }
    pub fn is_empty(&self) -> bool { self.ptr.is_null() || self.size == 0 }
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Alignment;

impl Alignment {
    pub const DEFAULT: usize = 16;
    pub const CACHE_LINE: usize = 64;
    pub fn is_power_of_two(v: usize) -> bool { v > 0 && (v & (v - 1)) == 0 }
    pub fn align_up(v: usize, a: usize) -> usize { (v + (a - 1)) & !(a - 1) }
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum MemoryError { InvalidArgument, Failed, OutOfMemory, NotSupported, NotLifo }
