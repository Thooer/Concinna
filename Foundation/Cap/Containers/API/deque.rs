use cap_memory::{Allocator, MemoryBlock, MemoryError};

pub struct Deque<'a, T: Copy> { ptr: *mut T, cap: usize, mask: usize, head: usize, tail: usize, blk: MemoryBlock, alloc: Allocator<'a> }

impl<'a, T: Copy> Deque<'a, T> {
    pub fn with_capacity(alloc: Allocator<'a>, capacity_pow2: usize) -> Result<Self, MemoryError> {
        if capacity_pow2 == 0 || (capacity_pow2 & (capacity_pow2 - 1)) != 0 { return Err(MemoryError::InvalidArgument); }
        let bytes = capacity_pow2.checked_mul(core::mem::size_of::<T>()).ok_or(MemoryError::Failed)?;
        let blk = alloc.alloc(bytes, core::mem::align_of::<T>())?;
        let ptr = blk.ptr.cast::<T>();
        Ok(Self { ptr, cap: capacity_pow2, mask: capacity_pow2 - 1, head: 0, tail: 0, blk, alloc })
    }
    pub fn is_empty(&self) -> bool { self.head == self.tail }
    pub fn is_full(&self) -> bool { (self.tail + 1) & self.mask == (self.head & self.mask) }
    pub fn push_back(&mut self, v: T) -> Result<(), MemoryError> { if self.is_full() { return Err(MemoryError::OutOfMemory); } unsafe { core::ptr::write(self.ptr.add(self.tail & self.mask), v); } self.tail = (self.tail + 1) & ((self.mask<<1)|1); Ok(()) }
    pub fn push_front(&mut self, v: T) -> Result<(), MemoryError> { if self.is_full() { return Err(MemoryError::OutOfMemory); } self.head = self.head.wrapping_sub(1) & ((self.mask<<1)|1); unsafe { core::ptr::write(self.ptr.add(self.head & self.mask), v); } Ok(()) }
    pub fn pop_front(&mut self) -> Option<T> { if self.is_empty() { None } else { let i = self.head & self.mask; let v = unsafe { core::ptr::read(self.ptr.add(i)) }; self.head = (self.head + 1) & ((self.mask<<1)|1); Some(v) } }
    pub fn pop_back(&mut self) -> Option<T> { if self.is_empty() { None } else { self.tail = self.tail.wrapping_sub(1) & ((self.mask<<1)|1); let i = self.tail & self.mask; Some(unsafe { core::ptr::read(self.ptr.add(i)) }) } }
    pub fn len(&self) -> usize { (self.tail.wrapping_sub(self.head)) & (((self.mask<<1)|1)) }
    pub fn get(&self, index: usize) -> Option<T> { let n = self.len(); if index < n { let i = (self.head.wrapping_add(index)) & self.mask; Some(unsafe { core::ptr::read(self.ptr.add(i)) }) } else { None } }
}

impl<'a, T: Copy> Drop for Deque<'a, T> { fn drop(&mut self) { if !self.blk.is_empty() { self.alloc.free(self.blk, core::mem::align_of::<T>()); } } }
