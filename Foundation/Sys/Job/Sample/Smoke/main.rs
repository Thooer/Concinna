use cap_memory::*;
use sys_job::{Scheduler, Job, TaskGroup};
use cap_concurrency::FiberStackPool;
use core::ffi::c_void;
use core::sync::atomic::{AtomicUsize, Ordering};

static SUM: AtomicUsize = AtomicUsize::new(0);

fn job_add(arg: *mut c_void) {
    let tg: &TaskGroup = unsafe { &*(arg as *const TaskGroup) };
    SUM.fetch_add(1, Ordering::Relaxed);
    tg.task_done();
}

fn main() {
    let mut frame = FrameAllocatorResource::new(8 << 20);
    let a = Allocator::new(&mut frame);
    let pool = FiberStackPool::new(64 << 10);
    println!("start");
    let sched = Scheduler::start(a, 3, 16, pool.stack_size()).unwrap();
    let tg = TaskGroup::new();

    let ptr_tg = &tg as *const _ as *mut c_void;
    let n = 2usize;
    tg.add_tasks(n);
    println!("enqueue");
    for _ in 0..n { let _ = sched.enqueue(Job::new(job_add as fn(*mut c_void), ptr_tg)); }

    println!("wait");
    tg.wait();
    println!("sum {}", SUM.load(Ordering::Relaxed));
}
