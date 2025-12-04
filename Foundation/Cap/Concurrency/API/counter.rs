use core::sync::atomic::{AtomicUsize, Ordering};

pub struct Counter { v: AtomicUsize }

impl Counter {
    pub const fn new() -> Self { Self { v: AtomicUsize::new(0) } }
    pub fn get(&self) -> usize { self.v.load(Ordering::Acquire) }
    pub fn add(&self, n: usize) { self.v.fetch_add(n, Ordering::Release); }
    pub fn sub(&self, n: usize) {
        let prev = self.v.fetch_sub(n, Ordering::AcqRel);
        if prev <= n { let p = &self.v as *const AtomicUsize as *mut u8; prm_threading::wake_by_address_all(p); }
    }
    pub fn wait_zero(&self, timeout_ms: u32) -> bool {
        loop {
            let cur = self.v.load(Ordering::Acquire);
            if cur == 0 { return true; }
            let p = &self.v as *const AtomicUsize as *const u8;
            let cmp = &cur as *const usize as *const u8;
            if prm_threading::wait_on_address(p, cmp, core::mem::size_of::<usize>(), timeout_ms).is_err() { return false; }
        }
    }
}
