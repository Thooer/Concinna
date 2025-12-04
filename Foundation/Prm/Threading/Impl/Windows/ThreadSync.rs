#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;

type HANDLE = *mut c_void;
const INFINITE: u32 = 0xFFFFFFFF;
const WAIT_OBJECT_0: u32 = 0x00000000;

#[repr(C)]
struct GROUP_AFFINITY {
    mask_low: u64,
    mask_high: u64,
    group: u16,
    reserved: [u16; 3],
}

#[link(name = "kernel32")]
extern "system" {
    fn CreateThread(lpThreadAttributes: *mut c_void, dwStackSize: usize, lpStartAddress: extern "system" fn(*mut c_void) -> u32, lpParameter: *mut c_void, dwCreationFlags: u32, lpThreadId: *mut u32) -> HANDLE;
    fn WaitForSingleObject(hHandle: HANDLE, dwMilliseconds: u32) -> u32;
    fn CloseHandle(hObject: HANDLE) -> i32;
    fn SwitchToThread() -> i32;
    fn Sleep(dwMilliseconds: u32);
    fn SetThreadAffinityMask(hThread: HANDLE, dwThreadAffinityMask: usize) -> usize;
    fn SetThreadGroupAffinity(hThread: HANDLE, groupAffinity: *const GROUP_AFFINITY, previousGroupAffinity: *mut GROUP_AFFINITY) -> i32;

    fn CreateMutexW(lpMutexAttributes: *mut c_void, bInitialOwner: i32, lpName: *const u16) -> HANDLE;
    fn ReleaseMutex(hMutex: HANDLE) -> i32;
    fn WaitForSingleObjectEx(hHandle: HANDLE, dwMilliseconds: u32, bAlertable: i32) -> u32;

    fn CreateSemaphoreW(lpSemaphoreAttributes: *mut c_void, lInitialCount: i32, lMaximumCount: i32, lpName: *const u16) -> HANDLE;
    fn ReleaseSemaphore(hSemaphore: HANDLE, lReleaseCount: i32, lpPreviousCount: *mut i32) -> i32;

    fn CreateEventW(lpEventAttributes: *mut c_void, bManualReset: i32, bInitialState: i32, lpName: *const u16) -> HANDLE;
    fn SetEvent(hEvent: HANDLE) -> i32;
    fn ResetEvent(hEvent: HANDLE) -> i32;

    fn WaitOnAddress(Address: *const c_void, CompareAddress: *const c_void, AddressSize: usize, dwMilliseconds: u32) -> i32;
    fn WakeByAddressSingle(Address: *mut c_void);
    fn WakeByAddressAll(Address: *mut c_void);
}

use crate::thread_sync::*;

#[repr(C)]
struct ThreadStart { f: ThreadFunc, u: *mut c_void }

extern "system" fn thread_trampoline(param: *mut c_void) -> u32 {
    unsafe {
        let boxed: Box<ThreadStart> = Box::from_raw(param as *mut ThreadStart);
        let f = boxed.f;
        let u = boxed.u;
        f(u);
    }
    0
}

pub fn impl_thread_create(f: ThreadFunc, u: *mut c_void) -> Result<ThreadHandle, ThreadingError> {
    let start = Box::new(ThreadStart { f, u });
    let param = Box::into_raw(start) as *mut c_void;
    let h = unsafe { CreateThread(std::ptr::null_mut(), 0, thread_trampoline, param, 0, std::ptr::null_mut()) };
    if h.is_null() { return Err(ThreadingError::Failed); }
    Ok(ThreadHandle(h))
}

pub fn impl_thread_create_with_stack(stack_size: usize, f: ThreadFunc, u: *mut c_void) -> Result<ThreadHandle, ThreadingError> {
    let start = Box::new(ThreadStart { f, u });
    let param = Box::into_raw(start) as *mut c_void;
    let h = unsafe { CreateThread(std::ptr::null_mut(), stack_size, thread_trampoline, param, 0, std::ptr::null_mut()) };
    if h.is_null() { return Err(ThreadingError::Failed); }
    Ok(ThreadHandle(h))
}

pub fn impl_thread_join(h: ThreadHandle) -> Result<(), ThreadingError> {
    let r = unsafe { WaitForSingleObject(h.0, INFINITE) };
    if r != WAIT_OBJECT_0 { return Err(ThreadingError::Failed); }
    unsafe { CloseHandle(h.0); }
    Ok(())
}

pub fn impl_thread_yield() { unsafe { SwitchToThread(); } }
pub fn impl_thread_sleep_ms(ms: u32) { unsafe { Sleep(ms) } }

pub fn impl_set_affinity_mask(h: ThreadHandle, mask: u64) -> Result<(), ThreadingError> {
    let r = unsafe { SetThreadAffinityMask(h.0, mask as usize) };
    if r == 0 { Err(ThreadingError::Failed) } else { Ok(()) }
}

pub fn impl_set_group_affinity(h: ThreadHandle, group: u16, mask: u64) -> Result<(), ThreadingError> {
    let ga = GROUP_AFFINITY { mask_low: mask, mask_high: 0, group, reserved: [0; 3] };
    let ok = unsafe { SetThreadGroupAffinity(h.0, &ga as *const GROUP_AFFINITY, std::ptr::null_mut()) };
    if ok == 0 { Err(ThreadingError::Failed) } else { Ok(()) }
}

pub fn impl_mutex_create() -> Result<MutexHandle, ThreadingError> {
    let h = unsafe { CreateMutexW(std::ptr::null_mut(), 0, std::ptr::null()) };
    if h.is_null() { Err(ThreadingError::Failed) } else { Ok(MutexHandle(h)) }
}

pub fn impl_mutex_destroy(h: MutexHandle) -> Result<(), ThreadingError> { unsafe { CloseHandle(h.0); } Ok(()) }

pub fn impl_mutex_lock(h: MutexHandle) -> Result<(), ThreadingError> {
    let r = unsafe { WaitForSingleObjectEx(h.0, INFINITE, 0) };
    if r != WAIT_OBJECT_0 { Err(ThreadingError::Failed) } else { Ok(()) }
}

pub fn impl_mutex_unlock(h: MutexHandle) -> Result<(), ThreadingError> { let ok = unsafe { ReleaseMutex(h.0) }; if ok == 0 { Err(ThreadingError::Failed) } else { Ok(()) } }

pub fn impl_mutex_try_lock(h: MutexHandle) -> Result<bool, ThreadingError> {
    let r = unsafe { WaitForSingleObjectEx(h.0, 0, 0) };
    if r == WAIT_OBJECT_0 { Ok(true) } else { Ok(false) }
}

pub fn impl_semaphore_create(initial: u32, max: u32) -> Result<SemaphoreHandle, ThreadingError> {
    let h = unsafe { CreateSemaphoreW(std::ptr::null_mut(), initial as i32, max as i32, std::ptr::null()) };
    if h.is_null() { Err(ThreadingError::Failed) } else { Ok(SemaphoreHandle(h)) }
}

pub fn impl_semaphore_destroy(h: SemaphoreHandle) -> Result<(), ThreadingError> { unsafe { CloseHandle(h.0); } Ok(()) }

pub fn impl_semaphore_acquire(h: SemaphoreHandle, timeout_ms: u32) -> Result<(), ThreadingError> {
    let r = unsafe { WaitForSingleObject(h.0, timeout_ms) };
    if r != WAIT_OBJECT_0 { Err(ThreadingError::Failed) } else { Ok(()) }
}

pub fn impl_semaphore_release(h: SemaphoreHandle, count: u32) -> Result<(), ThreadingError> {
    let ok = unsafe { ReleaseSemaphore(h.0, count as i32, std::ptr::null_mut()) };
    if ok == 0 { Err(ThreadingError::Failed) } else { Ok(()) }
}

pub fn impl_event_create(manual_reset: bool, initial_state: bool) -> Result<EventHandle, ThreadingError> {
    let h = unsafe { CreateEventW(std::ptr::null_mut(), manual_reset as i32, initial_state as i32, std::ptr::null()) };
    if h.is_null() { Err(ThreadingError::Failed) } else { Ok(EventHandle(h)) }
}

pub fn impl_event_destroy(h: EventHandle) -> Result<(), ThreadingError> { unsafe { CloseHandle(h.0); } Ok(()) }

pub fn impl_event_wait(h: EventHandle, timeout_ms: u32) -> Result<(), ThreadingError> {
    let r = unsafe { WaitForSingleObject(h.0, timeout_ms) };
    if r != WAIT_OBJECT_0 { Err(ThreadingError::Failed) } else { Ok(()) }
}

pub fn impl_event_signal(h: EventHandle) -> Result<(), ThreadingError> { let ok = unsafe { SetEvent(h.0) }; if ok == 0 { Err(ThreadingError::Failed) } else { Ok(()) } }
pub fn impl_event_reset(h: EventHandle) -> Result<(), ThreadingError> { let ok = unsafe { ResetEvent(h.0) }; if ok == 0 { Err(ThreadingError::Failed) } else { Ok(()) } }

pub fn impl_wait_on_address(addr: *const u8, cmp: *const u8, size: usize, timeout_ms: u32) -> Result<(), ThreadingError> {
    let ok = unsafe { WaitOnAddress(addr as *const c_void, cmp as *const c_void, size, timeout_ms) };
    if ok == 0 { Err(ThreadingError::Failed) } else { Ok(()) }
}

pub fn impl_wake_by_address_single(addr: *mut u8) { unsafe { WakeByAddressSingle(addr as *mut c_void) } }
pub fn impl_wake_by_address_all(addr: *mut u8) { unsafe { WakeByAddressAll(addr as *mut c_void) } }
