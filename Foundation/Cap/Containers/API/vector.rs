use core::mem::{size_of, MaybeUninit};
use core::ptr::{copy_nonoverlapping, write, read};
use cap_memory::{Allocator, MemoryBlock, MemoryError, Alignment};

pub struct Vector<'a, T: Copy> { ptr: *mut T, len: usize, cap: usize, blk: MemoryBlock, alloc: Allocator<'a> }

impl<'a, T: Copy> Vector<'a, T> {
    pub fn with_capacity(alloc: Allocator<'a>, capacity: usize) -> Result<Self, MemoryError> {
        let bytes = capacity.checked_mul(size_of::<T>()).ok_or(MemoryError::Failed)?;
        let blk = if bytes == 0 { MemoryBlock::empty() } else { alloc.alloc(bytes, core::mem::align_of::<T>())? };
        let ptr = if blk.is_empty() { core::ptr::null_mut() } else { blk.ptr.cast::<T>() };
        Ok(Self { ptr, len: 0, cap: capacity, blk, alloc })
    }
    pub fn with_capacity_aligned(alloc: Allocator<'a>, capacity: usize, align: usize) -> Result<Self, MemoryError> {
        let bytes = capacity.checked_mul(size_of::<T>()).ok_or(MemoryError::Failed)?;
        let blk = if bytes == 0 { MemoryBlock::empty() } else { alloc.alloc(bytes, align)? };
        let ptr = if blk.is_empty() { core::ptr::null_mut() } else { blk.ptr.cast::<T>() };
        Ok(Self { ptr, len: 0, cap: capacity, blk, alloc })
    }
    pub fn len(&self) -> usize { self.len }
    pub fn capacity(&self) -> usize { self.cap }
    pub fn as_ptr(&self) -> *const T { self.ptr as *const T }
    pub fn as_slice(&self) -> &[T] { unsafe { core::slice::from_raw_parts(self.ptr, self.len) } }
    pub fn as_mut_slice(&mut self) -> &mut [T] { unsafe { core::slice::from_raw_parts_mut(self.ptr, self.len) } }
    fn grow(&mut self) -> Result<(), MemoryError> {
        let new_cap = core::cmp::max(1, self.cap.checked_mul(2).ok_or(MemoryError::Failed)?);
        let new_bytes = new_cap.checked_mul(size_of::<T>()).ok_or(MemoryError::Failed)?;
        let nb = if self.blk.is_empty() { self.alloc.alloc(new_bytes, core::mem::align_of::<T>())? } else { self.alloc.realloc(self.blk, new_bytes, core::mem::align_of::<T>())? };
        self.ptr = nb.ptr.cast::<T>();
        self.cap = new_cap;
        self.blk = nb;
        Ok(())
    }
    pub fn push(&mut self, v: T) -> Result<(), MemoryError> {
        if self.len == self.cap { self.grow()?; }
        unsafe { write(self.ptr.add(self.len), v); }
        self.len += 1;
        Ok(())
    }
    pub fn pop(&mut self) -> Option<T> {
        if self.len == 0 { return None; }
        self.len -= 1;
        Some(unsafe { read(self.ptr.add(self.len)) })
    }
    pub fn get(&self, i: usize) -> Option<T> { if i < self.len { Some(unsafe { read(self.ptr.add(i)) }) } else { None } }
}

impl<'a, T: Copy> Drop for Vector<'a, T> {
    fn drop(&mut self) { if !self.blk.is_empty() { self.alloc.free(self.blk, core::mem::align_of::<T>()); } }
}
