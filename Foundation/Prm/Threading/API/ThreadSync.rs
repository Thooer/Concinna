#[path = "Types.rs"]
mod types;
pub use types::*;
use std::ffi::c_void;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/ThreadSync.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub fn impl_thread_create(_f: ThreadFunc, _u: *mut c_void) -> Result<ThreadHandle, ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_thread_create_with_stack(_stack_size: usize, _f: ThreadFunc, _u: *mut c_void) -> Result<ThreadHandle, ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_thread_join(_h: ThreadHandle) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_thread_yield() {}
    pub fn impl_thread_sleep_ms(_ms: u32) {}
    pub fn impl_set_affinity_mask(_h: ThreadHandle, _mask: u64) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_set_group_affinity(_h: ThreadHandle, _group: u16, _mask: u64) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_mutex_create() -> Result<MutexHandle, ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_mutex_destroy(_h: MutexHandle) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_mutex_lock(_h: MutexHandle) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_mutex_unlock(_h: MutexHandle) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_mutex_try_lock(_h: MutexHandle) -> Result<bool, ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_semaphore_create(_initial: u32, _max: u32) -> Result<SemaphoreHandle, ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_semaphore_destroy(_h: SemaphoreHandle) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_semaphore_acquire(_h: SemaphoreHandle, _timeout_ms: u32) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_semaphore_release(_h: SemaphoreHandle, _count: u32) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_event_create(_manual_reset: bool, _initial_state: bool) -> Result<EventHandle, ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_event_destroy(_h: EventHandle) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_event_wait(_h: EventHandle, _timeout_ms: u32) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_event_signal(_h: EventHandle) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_event_reset(_h: EventHandle) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_wait_on_address(_addr: *const u8, _cmp: *const u8, _size: usize, _timeout_ms: u32) -> Result<(), ThreadingError> { Err(ThreadingError::Unsupported) }
    pub fn impl_wake_by_address_single(_addr: *mut u8) {}
    pub fn impl_wake_by_address_all(_addr: *mut u8) {}
}

pub fn thread_create(f: ThreadFunc, u: *mut c_void) -> Result<ThreadHandle, ThreadingError> { backend::impl_thread_create(f, u) }
pub fn thread_create_with_stack(stack_size: usize, f: ThreadFunc, u: *mut c_void) -> Result<ThreadHandle, ThreadingError> { backend::impl_thread_create_with_stack(stack_size, f, u) }
pub fn thread_join(h: ThreadHandle) -> Result<(), ThreadingError> { backend::impl_thread_join(h) }
pub fn thread_yield() { backend::impl_thread_yield() }
pub fn thread_sleep_ms(ms: u32) { backend::impl_thread_sleep_ms(ms) }
pub fn thread_set_affinity_mask(h: ThreadHandle, mask: u64) -> Result<(), ThreadingError> { backend::impl_set_affinity_mask(h, mask) }
pub fn thread_set_group_affinity(h: ThreadHandle, group: u16, mask: u64) -> Result<(), ThreadingError> { backend::impl_set_group_affinity(h, group, mask) }
pub fn mutex_create() -> Result<MutexHandle, ThreadingError> { backend::impl_mutex_create() }
pub fn mutex_destroy(h: MutexHandle) -> Result<(), ThreadingError> { backend::impl_mutex_destroy(h) }
pub fn mutex_lock(h: MutexHandle) -> Result<(), ThreadingError> { backend::impl_mutex_lock(h) }
pub fn mutex_unlock(h: MutexHandle) -> Result<(), ThreadingError> { backend::impl_mutex_unlock(h) }
pub fn mutex_try_lock(h: MutexHandle) -> Result<bool, ThreadingError> { backend::impl_mutex_try_lock(h) }
pub fn semaphore_create(initial: u32, max: u32) -> Result<SemaphoreHandle, ThreadingError> { backend::impl_semaphore_create(initial, max) }
pub fn semaphore_destroy(h: SemaphoreHandle) -> Result<(), ThreadingError> { backend::impl_semaphore_destroy(h) }
pub fn semaphore_acquire(h: SemaphoreHandle, timeout_ms: u32) -> Result<(), ThreadingError> { backend::impl_semaphore_acquire(h, timeout_ms) }
pub fn semaphore_release(h: SemaphoreHandle, count: u32) -> Result<(), ThreadingError> { backend::impl_semaphore_release(h, count) }
pub fn event_create(manual_reset: bool, initial_state: bool) -> Result<EventHandle, ThreadingError> { backend::impl_event_create(manual_reset, initial_state) }
pub fn event_destroy(h: EventHandle) -> Result<(), ThreadingError> { backend::impl_event_destroy(h) }
pub fn event_wait(h: EventHandle, timeout_ms: u32) -> Result<(), ThreadingError> { backend::impl_event_wait(h, timeout_ms) }
pub fn event_signal(h: EventHandle) -> Result<(), ThreadingError> { backend::impl_event_signal(h) }
pub fn event_reset(h: EventHandle) -> Result<(), ThreadingError> { backend::impl_event_reset(h) }
pub fn wait_on_address(addr: *const u8, cmp: *const u8, size: usize, timeout_ms: u32) -> Result<(), ThreadingError> { backend::impl_wait_on_address(addr, cmp, size, timeout_ms) }
pub fn wake_by_address_single(addr: *mut u8) { backend::impl_wake_by_address_single(addr) }
pub fn wake_by_address_all(addr: *mut u8) { backend::impl_wake_by_address_all(addr) }
