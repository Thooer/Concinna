use prm_threading::*;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::Arc;

fn worker_counter(arg: *mut std::ffi::c_void) {
    let arc = unsafe { Arc::<AtomicUsize>::from_raw(arg as *const AtomicUsize) };
    for _ in 0..10000 { arc.fetch_add(1, Ordering::Relaxed); }
    // drop one Arc ref by not converting back; main holds the other refs
}

fn main() {
    let counter = Arc::new(AtomicUsize::new(0));
    let p1 = Arc::into_raw(counter.clone()) as *mut _;
    let p2 = Arc::into_raw(counter.clone()) as *mut _;
    let t1 = thread_create(worker_counter as ThreadFunc, p1).unwrap();
    let t2 = thread_create(worker_counter as ThreadFunc, p2).unwrap();
    thread_join(t1).unwrap();
    thread_join(t2).unwrap();
    println!("count={}", counter.load(Ordering::Relaxed));
    ensure_thread_is_fiber();
    fn fiber_fn(_arg: *mut std::ffi::c_void) {}
    let f = create_fiber(0, fiber_fn as ThreadFunc, std::ptr::null_mut()).unwrap();
    switch_to_fiber(f);
    delete_fiber(f);
}
