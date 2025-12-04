use prm_sync::{SpinLock, ScopedLock};
use std::sync::Arc;
use std::thread;

fn main() {
    let lock = Arc::new(SpinLock::new());
    let counter = Arc::new(std::sync::atomic::AtomicUsize::new(0));

    let mut handles = Vec::new();
    for _ in 0..4 {
        let l = lock.clone();
        let c = counter.clone();
        handles.push(thread::spawn(move || {
            for _ in 0..10000 {
                let _g = ScopedLock::new(&l);
                c.fetch_add(1, std::sync::atomic::Ordering::Relaxed);
            }
        }));
    }
    for h in handles { h.join().unwrap(); }
    let v = counter.load(std::sync::atomic::Ordering::Relaxed);
    assert!(v == 40000);
    println!("OK: {}", v);
}

