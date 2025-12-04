#![allow(unused_imports)]
use core::ffi::c_void;
use prm_threading::{FiberHandle, ThreadFunc, ThreadingError};
extern crate alloc;
use alloc::boxed::Box;

pub struct Fiber {
    pub handle: FiberHandle,
    pub owner: *mut c_void,
    pub prio: u8,
    pub ret: FiberHandle,
}

struct FiberCtx { func: fn(*mut c_void), arg: *mut c_void }

fn fiber_entry(ctx: *mut c_void) {
    unsafe {
        let b: Box<FiberCtx> = Box::from_raw(ctx as *mut FiberCtx);
        (b.func)(b.arg);
        let host = prm_threading::host_fiber();
        prm_threading::switch_to_fiber(host);
    }
}

impl Fiber {
    pub fn setup(func: fn(*mut c_void), arg: *mut c_void, _host: FiberHandle, stack_size: usize) -> Result<Self, ThreadingError> {
        let param = Box::into_raw(Box::new(FiberCtx { func, arg })) as *mut c_void;
        let h = prm_threading::create_fiber(stack_size, fiber_entry as ThreadFunc, param)?;
        Ok(Fiber { handle: h, owner: core::ptr::null_mut(), prio: 0, ret: _host })
    }
    pub fn start_switch(&self) { prm_threading::switch_to_fiber(self.handle) }
}

impl Drop for Fiber { fn drop(&mut self) { prm_threading::delete_fiber(self.handle); } }
