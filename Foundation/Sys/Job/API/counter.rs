use core::sync::atomic::{AtomicUsize, Ordering};
use cap_concurrency::Parker;
use prm_threading::wait_on_address;

pub struct TaskGroup { n: AtomicUsize, p: Parker }

impl TaskGroup {
    pub fn new() -> Self { Self { n: AtomicUsize::new(0), p: Parker::new().unwrap() } }
    pub fn add_tasks(&self, n: usize) { self.n.fetch_add(n, Ordering::Relaxed); let _ = self.p.reset(); }
    pub fn task_done(&self) { if self.n.fetch_sub(1, Ordering::Relaxed) == 1 { let _ = self.p.unpark(); } }
    pub fn wait(&self) {
        loop {
            if self.n.load(Ordering::Acquire) == 0 { break; }
            let _ = self.p.park(1000);
        }
    }
    pub fn is_done(&self) -> bool { self.n.load(Ordering::Acquire) == 0 }
}
