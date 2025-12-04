use core::ffi::c_void;
use core::sync::atomic::{AtomicBool, Ordering};
use cap_memory::*;
use cap_containers::*;
use cap_concurrency::{fiber, Parker};
use crate::job::Job;
use crate::driver::{Driver, set_resume_cb};
use prm_threading::{ThreadHandle, thread_create_with_stack, thread_join};

thread_local! { static CURRENT_FIBER: std::cell::Cell<*mut fiber::Fiber> = std::cell::Cell::new(core::ptr::null_mut()); }

pub fn suspend_current() { let host = prm_threading::host_fiber(); CURRENT_FIBER.with(|c| { let f = c.get(); if !f.is_null() { prm_threading::switch_to_fiber(host); } }); }
pub fn resume_fiber(fb: *mut fiber::Fiber) { unsafe { (*fb).start_switch(); } }

struct Worker<'a> { high: ChaseLevDeque<'a, Job>, norm: ChaseLevDeque<'a, Job>, parker: Parker }

struct WorkerCtx<'a> { self_idx: usize, workers: *mut Worker<'a>, worker_count: usize, stop: *const AtomicBool, stack_size: usize, driver_ptr: *mut Driver }

fn worker_main<'a>(arg: *mut c_void) {
    let ctx: &WorkerCtx = unsafe { &*(arg as *const WorkerCtx) };
    loop {
        let should_stop = unsafe { (*ctx.stop).load(Ordering::Relaxed) };
        if should_stop { break; }
        let w: &Worker = unsafe { &*ctx.workers.add(ctx.self_idx) };
        fn noop(_: *mut c_void) {}
        let mut j: Job = Job { func: noop, arg: core::ptr::null_mut(), qos: 0 };
        let got = if w.high.steal(&mut j) || w.high.pop_bottom(&mut j) { true } else if w.norm.steal(&mut j) || w.norm.pop_bottom(&mut j) { true } else {
            let mut found = false;
            for i in 0..ctx.worker_count { if i == ctx.self_idx { continue; } let other: &Worker = unsafe { &*ctx.workers.add(i) }; if other.high.steal(&mut j) || other.norm.steal(&mut j) { found = true; break; } }
            found
        };
        if got {
            println!("worker {} got job", ctx.self_idx);
            let host = prm_threading::host_fiber();
            struct FiberScope(*mut fiber::Fiber);
            impl FiberScope { fn new(func: fn(*mut c_void), arg: *mut c_void, host: prm_threading::FiberHandle, stack: usize) -> Self { let f = fiber::Fiber::setup(func, arg, host, stack).unwrap(); let fb = Box::into_raw(Box::new(f)) as *mut fiber::Fiber; CURRENT_FIBER.with(|c| c.set(fb)); Self(fb) } fn run(&self) { unsafe { (*self.0).start_switch(); } } }
            impl Drop for FiberScope { fn drop(&mut self) { CURRENT_FIBER.with(|c| c.set(core::ptr::null_mut())); unsafe { let _ = Box::from_raw(self.0); } } }
            let scope = FiberScope::new(j.func, j.arg, host, ctx.stack_size);
            scope.run();
            println!("worker {} finished job", ctx.self_idx);
        } else {
            unsafe { (*ctx.driver_ptr).poll(); }
            let timeout = unsafe { (*ctx.driver_ptr).next_timeout_ms() };
            let _ = w.parker.park(timeout);
        }
    }
}

pub struct Scheduler<'a> { alloc: Allocator<'a>, workers: *mut Worker<'a>, workers_blk: MemoryBlock, worker_count: usize, stop: AtomicBool, driver_ptr: *mut Driver, driver_blk: MemoryBlock, stack_size: usize, handles: *mut ThreadHandle, handles_blk: MemoryBlock }

impl<'a> Scheduler<'a> {
    pub fn start(alloc: Allocator<'a>, worker_count: usize, deque_capacity: usize, stack_size: usize) -> Result<Self, MemoryError> {
        let ws_blk = alloc.alloc(core::mem::size_of::<Worker>() * worker_count, core::mem::align_of::<Worker>())?;
        let workers = ws_blk.ptr as *mut Worker;
        for i in 0..worker_count {
            unsafe {
                workers.add(i).write(Worker { high: ChaseLevDeque::with_capacity(alloc, deque_capacity)?, norm: ChaseLevDeque::with_capacity(alloc, deque_capacity)?, parker: Parker::new().map_err(|_| MemoryError::Failed)? });
            }
        }
        let d_blk = alloc.alloc(core::mem::size_of::<Driver>(), core::mem::align_of::<Driver>())?;
        let d_ptr = d_blk.ptr.cast::<Driver>();
        unsafe { core::ptr::write(d_ptr, Driver::new()); }
        let h_blk = alloc.alloc(core::mem::size_of::<ThreadHandle>() * worker_count, core::mem::align_of::<ThreadHandle>())?;
        let handles = h_blk.ptr.cast::<ThreadHandle>();
        let s = Self { alloc, workers, workers_blk: ws_blk, worker_count, stop: AtomicBool::new(false), driver_ptr: d_ptr, driver_blk: d_blk, stack_size, handles, handles_blk: h_blk };
        set_resume_cb(resume_fiber);
        for i in 0..worker_count {
            let ctx = Box::new(WorkerCtx { self_idx: i, workers, worker_count, stop: &s.stop, stack_size, driver_ptr: s.driver_ptr });
            let arg = Box::into_raw(ctx) as *mut c_void;
            let h = match thread_create_with_stack(8 << 20, worker_main as fn(*mut c_void), arg) { Ok(h) => h, Err(_) => return Err(MemoryError::Failed) };
            unsafe { s.handles.add(i).write(h); }
        }
        Ok(s)
    }
    pub fn stop(&mut self) -> Result<(), ()> {
        self.stop.store(true, Ordering::Relaxed);
        for i in 0..self.worker_count { unsafe { let w = &*self.workers.add(i); let _ = w.parker.unpark(); } }
        Ok(())
    }
    pub fn join(&mut self) {
        for i in 0..self.worker_count { unsafe { let h = self.handles.add(i).read(); let _ = thread_join(h); } }
    }
    pub fn enqueue(&self, j: Job) -> Result<(), MemoryError> {
        let idx = (j.qos as usize).min(self.worker_count - 1);
        unsafe { (*self.workers.add(idx)).norm.push_bottom(j); let _ = (*self.workers.add(idx)).parker.unpark(); }
        Ok(())
    }
    pub fn enqueue_high(&self, j: Job) -> Result<(), MemoryError> {
        let idx = (j.qos as usize).min(self.worker_count - 1);
        unsafe { (*self.workers.add(idx)).high.push_bottom(j); let _ = (*self.workers.add(idx)).parker.unpark(); }
        Ok(())
    }
    pub fn worker_count(&self) -> usize { self.worker_count }
}

impl<'a> Drop for Scheduler<'a> {
    fn drop(&mut self) {
        let _ = self.stop();
        self.join();
        if !self.workers_blk.is_empty() { self.alloc.free(self.workers_blk, core::mem::align_of::<Worker>()); }
        if !self.handles_blk.is_empty() { self.alloc.free(self.handles_blk, core::mem::align_of::<ThreadHandle>()); }
        if !self.driver_blk.is_empty() { unsafe { core::ptr::drop_in_place(self.driver_ptr); } self.alloc.free(self.driver_blk, core::mem::align_of::<Driver>()); }
    }
}
