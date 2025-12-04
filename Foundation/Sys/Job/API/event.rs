use core::sync::atomic::{AtomicBool, AtomicPtr, Ordering};
use cap_concurrency::fiber::Fiber;

struct WaitNode { next: *mut WaitNode, fiber: *mut Fiber }

pub struct FiberEvent { signaled: AtomicBool, head: AtomicPtr<WaitNode> }

impl FiberEvent {
    pub fn new() -> Self { Self { signaled: AtomicBool::new(false), head: AtomicPtr::new(core::ptr::null_mut()) } }
    pub fn signal_all(&self) {
        self.signaled.store(true, Ordering::Release);
        loop {
            let h = self.head.swap(core::ptr::null_mut(), Ordering::AcqRel);
            if h.is_null() { break; }
            unsafe {
                let mut cur = h;
                while !cur.is_null() {
                    let fb = (*cur).fiber;
                    crate::scheduler::resume_fiber(fb);
                    let nx = (*cur).next;
                    let _ = Box::from_raw(cur);
                    cur = nx;
                }
            }
        }
    }
    pub fn await_one(&self, fiber: *mut Fiber) -> bool {
        if self.signaled.load(Ordering::Acquire) { return true; }
        let n = Box::into_raw(Box::new(WaitNode { next: core::ptr::null_mut(), fiber }));
        loop {
            let h = self.head.load(Ordering::Acquire);
            unsafe { (*n).next = h; }
            if self.head.compare_exchange(h, n, Ordering::Release, Ordering::Relaxed).is_ok() { break; }
        }
        false
    }
}
