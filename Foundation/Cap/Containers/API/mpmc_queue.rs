use core::cell::UnsafeCell;
use core::mem::{size_of, MaybeUninit};
use core::sync::atomic::{AtomicUsize, Ordering};
use cap_memory::{Allocator, MemoryBlock, MemoryError};

#[repr(C, align(16))]
struct Slot<T: Copy> { val: UnsafeCell<MaybeUninit<T>>, seq: AtomicUsize, _pad: [u8; 8] }

pub struct MPMCQueue<'a, T: Copy> { slots: *mut Slot<T>, cap: usize, mask: usize, head: AtomicUsize, tail: AtomicUsize, blk: MemoryBlock, alloc: Allocator<'a> }

unsafe impl<'a, T: Copy + Send> Send for MPMCQueue<'a, T> {}
unsafe impl<'a, T: Copy + Send> Sync for MPMCQueue<'a, T> {}

impl<'a, T: Copy> MPMCQueue<'a, T> {
    pub fn with_capacity(alloc: Allocator<'a>, capacity_pow2: usize) -> Result<Self, MemoryError> {
        if capacity_pow2 == 0 || (capacity_pow2 & (capacity_pow2 - 1)) != 0 { return Err(MemoryError::InvalidArgument); }
        let bytes = capacity_pow2.checked_mul(size_of::<Slot<T>>()).ok_or(MemoryError::Failed)?;
        let blk = alloc.alloc(bytes, core::mem::align_of::<Slot<T>>())?;
        let slots = blk.ptr.cast::<Slot<T>>();
        for i in 0..capacity_pow2 {
            unsafe {
                let s = slots.add(i);
                core::ptr::write(&mut (*s).val, UnsafeCell::new(MaybeUninit::uninit()));
                core::ptr::write(&mut (*s).seq, AtomicUsize::new(i));
            }
        }
        Ok(Self { slots, cap: capacity_pow2, mask: capacity_pow2 - 1, head: AtomicUsize::new(0), tail: AtomicUsize::new(0), blk, alloc })
    }
    pub fn enqueue(&self, v: T) -> Result<(), MemoryError> {
        loop {
            let t = self.tail.load(Ordering::Relaxed);
            let s = unsafe { &*self.slots.add(t & self.mask) };
            let seq = s.seq.load(Ordering::Acquire);
            let dif = seq as isize - t as isize;
            if dif == 0 {
                if self.tail.compare_exchange(t, t + 1, Ordering::Relaxed, Ordering::Relaxed).is_ok() {
                    unsafe { (*s).val.get().write(MaybeUninit::new(v)); }
                    s.seq.store(t + 1, Ordering::Release);
                    return Ok(());
                }
            } else if dif < 0 {
                return Err(MemoryError::OutOfMemory);
            }
        }
    }
    pub fn dequeue(&self) -> Option<T> {
        loop {
            let h = self.head.load(Ordering::Relaxed);
            let s = unsafe { &*self.slots.add(h & self.mask) };
            let seq = s.seq.load(Ordering::Acquire);
            let dif = seq as isize - (h as isize + 1);
            if dif == 0 {
                if self.head.compare_exchange(h, h + 1, Ordering::Relaxed, Ordering::Relaxed).is_ok() {
                    let v = unsafe { (*s).val.get().read().assume_init() };
                    s.seq.store(h + self.cap, Ordering::Release);
                    return Some(v);
                }
            } else if dif < 0 { return None; }
        }
    }
}

impl<'a, T: Copy> Drop for MPMCQueue<'a, T> { fn drop(&mut self) { if !self.blk.is_empty() { self.alloc.free(self.blk, core::mem::align_of::<Slot<T>>()); } } }
