use core::sync::atomic::{AtomicPtr, Ordering};
use cap_concurrency::fiber::Fiber;

pub struct FiberMutex { owner: AtomicPtr<Fiber>, waiters: AtomicPtr<Fiber> }

impl FiberMutex {
    pub fn new() -> Self { Self { owner: AtomicPtr::new(core::ptr::null_mut()), waiters: AtomicPtr::new(core::ptr::null_mut()) } }
    pub fn try_lock(&self, fiber: *mut Fiber) -> bool { self.owner.compare_exchange(core::ptr::null_mut(), fiber, Ordering::Acquire, Ordering::Relaxed).is_ok() }
    pub fn lock(&self, fiber: *mut Fiber) {
        if self.try_lock(fiber) { return; }
        loop {
            let h = self.waiters.load(Ordering::Acquire);
            if self.waiters.compare_exchange(h, fiber, Ordering::Release, Ordering::Relaxed).is_ok() { break; }
        }
        crate::scheduler::suspend_current();
    }
    pub fn unlock(&self) {
        let _ = self.owner.swap(core::ptr::null_mut(), Ordering::Release);
        let w = self.waiters.swap(core::ptr::null_mut(), Ordering::AcqRel);
        if !w.is_null() { crate::scheduler::resume_fiber(w); }
}
}
