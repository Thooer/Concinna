use core::ffi::c_void;
use core::sync::atomic::{AtomicU32, Ordering};
use cap_io::*;
use crate::scheduler::resume_fiber;
use cap_concurrency::fiber::Fiber;

pub type ResumeFiberFn = fn(*mut Fiber);

static mut RESUME_CB: Option<ResumeFiberFn> = None;

pub struct TimerItem { pub start_ms: u64, pub wait_ms: u64, pub fiber: *mut Fiber, pub cb: Option<fn(*mut c_void)>, pub ctx: *mut c_void }
pub struct EventItem { pub h: *mut c_void, pub fiber: *mut Fiber }
pub struct IocpMapItem { pub ov: *mut c_void, pub fiber: *mut Fiber }

pub struct Driver {
    pub timers: heapless::Vec<TimerItem, 64>,
    pub events: heapless::Vec<EventItem, 64>,
    pub iocp: Option<Iocp>,
    pub maps: heapless::Vec<IocpMapItem, 64>,
    pub poller: AtomicU32,
}

impl Driver {
    pub fn new() -> Self { Self { timers: heapless::Vec::new(), events: heapless::Vec::new(), iocp: attach(), maps: heapless::Vec::new(), poller: AtomicU32::new(0) } }
    fn rem_wait(&self, idx: usize, now: u64) -> u64 { self.timers[idx].wait_ms.saturating_sub(now.saturating_sub(self.timers[idx].start_ms)) }
    fn heap_up(&mut self, mut i: usize) {
        let now = now_ms();
        while i > 0 {
            let p = (i - 1) >> 1;
            if self.rem_wait(i, now) < self.rem_wait(p, now) { self.timers.swap(i, p); i = p; } else { break; }
        }
    }
    fn heap_down(&mut self, mut i: usize) {
        let now = now_ms();
        let len = self.timers.len();
        loop {
            let l = (i << 1) + 1;
            if l >= len { break; }
            let r = l + 1;
            let mut m = l;
            if r < len && self.rem_wait(r, now) < self.rem_wait(l, now) { m = r; }
            if self.rem_wait(m, now) < self.rem_wait(i, now) { self.timers.swap(i, m); i = m; } else { break; }
        }
    }
    pub fn next_timeout_ms(&self) -> u32 {
        if self.timers.is_empty() { 1 } else {
            let now = now_ms();
            let t = &self.timers[0];
            let elapsed = now.saturating_sub(t.start_ms);
            let rem = if elapsed >= t.wait_ms { 0 } else { t.wait_ms - elapsed };
            rem as u32
        }
    }
    pub fn add_timer(&mut self, fiber: *mut Fiber, delay_ms: u32) -> bool {
        if self.timers.push(TimerItem { start_ms: now_ms(), wait_ms: delay_ms as u64, fiber, cb: None, ctx: core::ptr::null_mut() }).is_err() { return false; }
        let i = self.timers.len() - 1;
        self.heap_up(i);
        true
    }
    pub fn add_timeout(&mut self, cb: fn(*mut c_void), ctx: *mut c_void, delay_ms: u32) -> bool {
        if self.timers.push(TimerItem { start_ms: now_ms(), wait_ms: delay_ms as u64, fiber: core::ptr::null_mut(), cb: Some(cb), ctx }).is_err() { return false; }
        let i = self.timers.len() - 1;
        self.heap_up(i);
        true
    }
    pub fn add_event(&mut self, h: *mut c_void, fiber: *mut Fiber) -> bool { self.events.push(EventItem { h, fiber }).is_ok() }
    pub fn attach_iocp(&mut self) -> bool { if self.iocp.is_some() { true } else { self.iocp = attach(); self.iocp.is_some() } }
    pub fn register_iocp_handle(&mut self, h: *mut c_void) -> bool { if let Some(ref i) = self.iocp { register_handle(i, h) } else { false } }
    pub fn await_iocp(&mut self, ov: *mut c_void, fiber: *mut Fiber) -> bool { self.maps.push(IocpMapItem { ov, fiber }).is_ok() }
    pub fn poll(&mut self) {
        if let Some(ref i) = self.iocp {
            if let Some((ov, _bytes, _key)) = get_queued(i, 0) {
                for idx in 0..self.maps.len() {
                    if self.maps[idx].ov == ov {
                        let fb = self.maps[idx].fiber;
                        self.maps.swap_remove(idx);
                        unsafe { if let Some(cb) = RESUME_CB { cb(fb) } else { resume_fiber(fb) } }
                        break;
                    }
                }
            }
        }
        let now = now_ms();
        while let Some(t) = self.timers.first() {
            let rem = t.wait_ms.saturating_sub(now.saturating_sub(t.start_ms));
            if rem == 0 {
                let it = self.timers.swap_remove(0);
                if let Some(cb) = it.cb { cb(it.ctx) } else { unsafe { if let Some(rcb) = RESUME_CB { rcb(it.fiber) } else { resume_fiber(it.fiber) } } }
                if !self.timers.is_empty() { self.heap_down(0); }
            } else { break; }
        }
    }
    pub fn pending_count(&self) -> u32 { (self.timers.len() + self.events.len() + self.maps.len()) as u32 }
}

pub fn set_resume_cb(cb: ResumeFiberFn) { unsafe { RESUME_CB = Some(cb); } }

fn now_ms() -> u64 { (prm_time::now() as u64) / 1_000_000 }
