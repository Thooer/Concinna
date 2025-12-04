use std::ffi::c_void;
use crate::thread_sync::ThreadingError;

#[derive(Clone, Copy)]
pub struct FiberHandle(pub *mut c_void);

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Fiber.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub fn impl_ensure_thread_is_fiber() {}
    pub fn impl_revert_fiber_to_thread() {}
    pub fn impl_create_fiber(_stack_size: usize, _f: crate::thread_sync::ThreadFunc, _u: *mut c_void) -> Result<FiberHandle, ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_switch_to_fiber(_fiber: FiberHandle) {}
    pub fn impl_delete_fiber(_fiber: FiberHandle) {}
}

pub fn ensure_thread_is_fiber() { backend::impl_ensure_thread_is_fiber() }
pub fn revert_fiber_to_thread() { backend::impl_revert_fiber_to_thread() }
pub fn create_fiber(stack_size: usize, f: crate::thread_sync::ThreadFunc, u: *mut c_void) -> Result<FiberHandle, ThreadingError> { backend::impl_create_fiber(stack_size, f, u) }
pub fn switch_to_fiber(fiber: FiberHandle) { backend::impl_switch_to_fiber(fiber) }
pub fn delete_fiber(fiber: FiberHandle) { backend::impl_delete_fiber(fiber) }
pub fn host_fiber() -> FiberHandle { backend::impl_host_fiber() }
