use std::ffi::c_void;
use crate::thread_sync::{ThreadingError, ThreadFunc};

extern "system" {
    fn ConvertThreadToFiber(lpParameter: *mut c_void) -> *mut c_void;
    fn ConvertFiberToThread() -> i32;
    fn CreateFiber(dwStackSize: usize, lpStartAddress: extern "system" fn(*mut c_void), lpParameter: *mut c_void) -> *mut c_void;
    fn SwitchToFiber(lpFiber: *mut c_void);
    fn DeleteFiber(lpFiber: *mut c_void);
}

thread_local! { static HOST_FIBER: std::cell::Cell<*mut c_void> = std::cell::Cell::new(std::ptr::null_mut()); }

struct FiberStart { f: ThreadFunc, u: *mut c_void }

extern "system" fn fiber_trampoline(ctx: *mut c_void) {
    unsafe {
        let b: Box<FiberStart> = Box::from_raw(ctx as *mut FiberStart);
        (b.f)(b.u);
    }
}

pub fn impl_ensure_thread_is_fiber() {
    HOST_FIBER.with(|c| {
        if c.get().is_null() {
            let p = unsafe { ConvertThreadToFiber(std::ptr::null_mut()) };
            c.set(p);
        }
    });
}

pub fn impl_revert_fiber_to_thread() { unsafe { let _ = ConvertFiberToThread(); } }

pub fn impl_create_fiber(stack_size: usize, f: ThreadFunc, u: *mut c_void) -> Result<super::FiberHandle, ThreadingError> {
    impl_ensure_thread_is_fiber();
    let param = Box::into_raw(Box::new(FiberStart { f, u }));
    let h = unsafe { CreateFiber(stack_size, fiber_trampoline, param as *mut c_void) };
    if h.is_null() { Err(ThreadingError::Failed) } else { Ok(super::FiberHandle(h)) }
}

pub fn impl_switch_to_fiber(fiber: super::FiberHandle) { unsafe { SwitchToFiber(fiber.0) } }

pub fn impl_delete_fiber(fiber: super::FiberHandle) { unsafe { DeleteFiber(fiber.0) } }

pub fn impl_host_fiber() -> super::FiberHandle {
    let h = HOST_FIBER.with(|c| c.get());
    super::FiberHandle(h)
}
