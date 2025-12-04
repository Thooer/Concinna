use core::sync::atomic::{AtomicIsize, Ordering};
use cap_memory::{Allocator, MemoryBlock, MemoryError};

pub struct ChaseLevDeque<'a, T: Copy> {
    ptr: *mut T,
    cap: isize,
    mask: isize,
    top: AtomicIsize,
    bottom: AtomicIsize,
    blk: MemoryBlock,
    alloc: Allocator<'a>,
}

impl<'a, T: Copy> ChaseLevDeque<'a, T> {
    pub fn with_capacity(alloc: Allocator<'a>, capacity_pow2: usize) -> Result<Self, MemoryError> {
        if capacity_pow2 == 0 || (capacity_pow2 & (capacity_pow2 - 1)) != 0 { return Err(MemoryError::InvalidArgument); }
        let bytes = capacity_pow2.checked_mul(core::mem::size_of::<T>()).ok_or(MemoryError::Failed)?;
        let blk = alloc.alloc(bytes, core::mem::align_of::<T>())?;
        let ptr = blk.ptr.cast::<T>();
        Ok(Self { ptr, cap: capacity_pow2 as isize, mask: (capacity_pow2 as isize) - 1, top: AtomicIsize::new(0), bottom: AtomicIsize::new(0), blk, alloc })
    }

    pub fn push_bottom(&self, v: T) -> bool {
        let b = self.bottom.load(Ordering::Relaxed);
        unsafe { core::ptr::write(self.ptr.add((b & self.mask) as usize), v); }
        self.bottom.store(b + 1, Ordering::Release);
        true
    }

    pub fn pop_bottom(&self, out: &mut T) -> bool {
        let b = self.bottom.load(Ordering::Relaxed) - 1;
        self.bottom.store(b, Ordering::Relaxed);
        let t = self.top.load(Ordering::Acquire);
        let size = b - t;
        if size < 0 {
            self.bottom.store(t, Ordering::Relaxed);
            return false;
        }
        let v = unsafe { core::ptr::read(self.ptr.add((b & self.mask) as usize)) };
        if size > 0 { *out = v; return true; }
        let ok = self.top.compare_exchange(t, t + 1, Ordering::SeqCst, Ordering::Relaxed).is_ok();
        self.bottom.store(t + 1, Ordering::Relaxed);
        if ok { *out = v; true } else { false }
    }

    pub fn steal(&self, out: &mut T) -> bool {
        let t = self.top.load(Ordering::Acquire);
        let b = self.bottom.load(Ordering::Acquire);
        let size = b - t;
        if size <= 0 { return false; }
        let v = unsafe { core::ptr::read(self.ptr.add((t & self.mask) as usize)) };
        if self.top.compare_exchange(t, t + 1, Ordering::SeqCst, Ordering::Relaxed).is_ok() { *out = v; true } else { false }
    }
}

impl<'a, T: Copy> Drop for ChaseLevDeque<'a, T> { fn drop(&mut self) { if !self.blk.is_empty() { self.alloc.free(self.blk, core::mem::align_of::<T>()); } } }
