use cap_concurrency::*;
use core::ffi::c_void;
use core::sync::atomic::{AtomicUsize, Ordering};

static SUM: AtomicUsize = AtomicUsize::new(0);

fn fiber_fn(_: *mut c_void) { SUM.fetch_add(1, Ordering::Relaxed); }

fn main() {
    prm_threading::ensure_thread_is_fiber();
    let host = prm_threading::host_fiber();
    let pool = FiberStackPool::new(64 << 10);
    let f = Fiber::setup(fiber_fn as fn(*mut c_void), core::ptr::null_mut(), host, pool.stack_size()).unwrap();
    f.start_switch();
    println!("sum {}", SUM.load(Ordering::Relaxed));
}
