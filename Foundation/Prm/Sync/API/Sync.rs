use std::sync::atomic::{AtomicBool, Ordering};
use std::hint::spin_loop;
use std::thread::yield_now;

pub const CACHE_LINE: usize = 64;

pub struct Backoff { step: u32 }

impl Backoff {
    pub fn new() -> Self { Self { step: 0 } }
    pub fn snooze(&mut self) {
        self.step = self.step.saturating_add(1);
        let reps = self.step.min(32);
        for _ in 0..reps { spin_loop(); }
        if self.step > 32 { yield_now(); }
    }
    pub fn reset(&mut self) { self.step = 0 }
}

pub struct SpinLock { flag: AtomicBool }

impl SpinLock {
    pub fn new() -> Self { Self { flag: AtomicBool::new(false) } }
    pub fn try_lock(&self) -> bool { self.flag.compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed).is_ok() }
    pub fn lock(&self) { let mut b = Backoff::new(); while !self.try_lock() { b.snooze(); } }
    pub fn unlock(&self) { self.flag.store(false, Ordering::Release) }
}

pub struct ScopedLock<'a> { l: &'a SpinLock }

impl<'a> ScopedLock<'a> { pub fn new(l: &'a SpinLock) -> Self { l.lock(); Self { l } } }

impl<'a> Drop for ScopedLock<'a> { fn drop(&mut self) { self.l.unlock(); } }
