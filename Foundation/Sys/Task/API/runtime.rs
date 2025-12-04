use core::ffi::c_void;
use core::sync::atomic::{AtomicBool, AtomicUsize, Ordering};
use cap_concurrency::Parker;
use prm_threading::{ThreadHandle, thread_create, thread_join};
use crate::task_graph::TaskGraph;
use crate::task::ThreadAffinity;
use sys_job::{Scheduler, TaskGroup};

struct RuntimeCtx<'a> { rt: *mut TaskRuntime<'a> }

fn runtime_entry<'a>(arg: *mut c_void) {
    let ctx: &RuntimeCtx = unsafe { &*(arg as *const RuntimeCtx) };
    let rt: &TaskRuntime = unsafe { &*ctx.rt };
    loop {
        if rt.stop.load(Ordering::Relaxed) { break; }
        let g = rt.graph;
        let tg = rt.tg;
        if !g.is_null() && !tg.is_null() {
            let done = unsafe { (*tg).is_done() };
            if done {
                let fi = rt.frame_idx.load(Ordering::Relaxed);
                let gd = rt.gpu_done.load(Ordering::Relaxed);
                if fi < rt.target_frames && fi.saturating_sub(gd) < rt.max_ahead {
                    let dispatched = unsafe { (*g).dispatch_next_frame_roots(&*rt.sched, &*tg) };
                    if dispatched > 0 { rt.frame_idx.fetch_add(1, Ordering::Relaxed); }
                }
            }
        }
        prm_threading::thread_sleep_ms(1);
    }
}

pub struct TaskRuntime<'a> { stop: AtomicBool, sched: *const Scheduler<'a>, graph: *mut TaskGraph<'a>, tg: *const TaskGroup, parker: Parker, handle: Option<ThreadHandle>, frame_idx: AtomicUsize, gpu_done: AtomicUsize, max_ahead: usize, target_frames: usize }

impl<'a> TaskRuntime<'a> {
    pub fn new(sched: &Scheduler<'a>) -> Self { Self { stop: AtomicBool::new(false), sched: sched as *const Scheduler<'a>, graph: core::ptr::null_mut(), tg: core::ptr::null(), parker: Parker::new().unwrap(), handle: None, frame_idx: AtomicUsize::new(0), gpu_done: AtomicUsize::new(0), max_ahead: 1, target_frames: 1 } }
    pub fn bind(&mut self, g: &mut TaskGraph<'a>, tg: &TaskGroup) { self.graph = g as *mut TaskGraph<'a>; self.tg = tg as *const TaskGroup; }
    pub fn configure(&mut self, max_ahead: usize, target_frames: usize) { self.max_ahead = max_ahead; self.target_frames = target_frames; }
    pub fn start(&mut self) { let ctx = Box::new(RuntimeCtx { rt: self as *const TaskRuntime<'a> as *mut TaskRuntime<'a> }); let arg = Box::into_raw(ctx) as *mut c_void; match thread_create(runtime_entry as fn(*mut c_void), arg) { Ok(h) => { self.handle = Some(h); } Err(_) => {} } }
    pub fn stop_and_join(&mut self) { self.stop.store(true, Ordering::Relaxed); let _ = self.parker.unpark(); if let Some(h) = self.handle.take() { let _ = thread_join(h); } }
    pub fn mark_gpu_completed(&self) { self.gpu_done.fetch_add(1, Ordering::Relaxed); }
}
