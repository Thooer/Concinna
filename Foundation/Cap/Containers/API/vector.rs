use core::mem::{size_of, MaybeUninit};
use core::ptr::{copy_nonoverlapping, write, read, drop_in_place};
use cap_memory::{Allocator, MemoryBlock, MemoryError, Alignment};
use core::ops::{Deref, DerefMut, Index, IndexMut};

pub struct Vector<'a, T> { ptr: *mut T, len: usize, cap: usize, blk: MemoryBlock, alloc: Allocator<'a> }

unsafe impl<'a, T: Send> Send for Vector<'a, T> {}
unsafe impl<'a, T: Sync> Sync for Vector<'a, T> {}

impl<'a, T> Vector<'a, T> {
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
    pub fn is_empty(&self) -> bool { self.len == 0 }
    pub fn capacity(&self) -> usize { self.cap }
    pub fn as_ptr(&self) -> *const T { self.ptr as *const T }
    pub fn as_mut_ptr(&mut self) -> *mut T { self.ptr }
    pub fn as_slice(&self) -> &[T] { unsafe { core::slice::from_raw_parts(self.ptr, self.len) } }
    pub fn as_mut_slice(&mut self) -> &mut [T] { unsafe { core::slice::from_raw_parts_mut(self.ptr, self.len) } }
    
    fn grow(&mut self) -> Result<(), MemoryError> {
        let new_cap = if self.cap == 0 { 4 } else { self.cap.checked_mul(2).ok_or(MemoryError::Failed)? };
        let new_bytes = new_cap.checked_mul(size_of::<T>()).ok_or(MemoryError::Failed)?;
        let nb = if self.blk.is_empty() { 
            self.alloc.alloc(new_bytes, core::mem::align_of::<T>())? 
        } else { 
            self.alloc.realloc(self.blk, new_bytes, core::mem::align_of::<T>())? 
        };
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
    
    pub fn swap_remove(&mut self, index: usize) -> Option<T> {
        if index >= self.len { return None; }
        unsafe {
            let last_idx = self.len - 1;
            let val = read(self.ptr.add(index));
            if index != last_idx {
                copy_nonoverlapping(self.ptr.add(last_idx), self.ptr.add(index), 1);
            }
            self.len -= 1;
            Some(val)
        }
    }

    pub fn get(&self, i: usize) -> Option<&T> { if i < self.len { Some(unsafe { &*self.ptr.add(i) }) } else { None } }
    pub fn get_mut(&mut self, i: usize) -> Option<&mut T> { if i < self.len { Some(unsafe { &mut *self.ptr.add(i) }) } else { None } }
}

impl<'a, T> Drop for Vector<'a, T> {
    fn drop(&mut self) {
        if !self.blk.is_empty() {
            unsafe {
                // Drop all elements
                let s = core::slice::from_raw_parts_mut(self.ptr, self.len);
                for x in s { drop_in_place(x); }
            }
            self.alloc.free(self.blk, core::mem::align_of::<T>());
        }
    }
}

impl<'a, T> Deref for Vector<'a, T> {
    type Target = [T];
    fn deref(&self) -> &Self::Target { self.as_slice() }
}

impl<'a, T> DerefMut for Vector<'a, T> {
    fn deref_mut(&mut self) -> &mut Self::Target { self.as_mut_slice() }
}

impl<'a, T> Index<usize> for Vector<'a, T> {
    type Output = T;
    fn index(&self, index: usize) -> &Self::Output { &self.as_slice()[index] }
}

impl<'a, T> IndexMut<usize> for Vector<'a, T> {
    fn index_mut(&mut self, index: usize) -> &mut Self::Output { &mut self.as_mut_slice()[index] }
}

impl<'a, T> IntoIterator for Vector<'a, T> {
    type Item = T;
    type IntoIter = IntoIter<'a, T>;
    fn into_iter(mut self) -> Self::IntoIter {
        let ptr = self.ptr;
        let len = self.len;
        let blk = self.blk;
        let alloc = self.alloc;
        // Prevent Drop from freeing the memory or dropping elements
        self.blk = MemoryBlock::empty(); 
        self.len = 0; // Prevent drop from dropping elements
        IntoIter { ptr, len, idx: 0, blk, alloc }
    }
}

pub struct IntoIter<'a, T> { ptr: *mut T, len: usize, idx: usize, blk: MemoryBlock, alloc: Allocator<'a> }

unsafe impl<'a, T: Send> Send for IntoIter<'a, T> {}
unsafe impl<'a, T: Sync> Sync for IntoIter<'a, T> {}

impl<'a, T> Iterator for IntoIter<'a, T> {
    type Item = T;
    fn next(&mut self) -> Option<Self::Item> {
        if self.idx < self.len {
            let v = unsafe { read(self.ptr.add(self.idx)) };
            self.idx += 1;
            Some(v)
        } else { None }
    }
}

impl<'a, T> Drop for IntoIter<'a, T> {
    fn drop(&mut self) {
        if !self.blk.is_empty() {
             unsafe {
                // Drop remaining elements
                if self.idx < self.len {
                     let s = core::slice::from_raw_parts_mut(self.ptr.add(self.idx), self.len - self.idx);
                     for x in s { drop_in_place(x); }
                }
            }
            self.alloc.free(self.blk, core::mem::align_of::<T>());
        }
    }
}
