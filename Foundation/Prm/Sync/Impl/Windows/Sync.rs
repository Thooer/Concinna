#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;

#[repr(C)]
struct CRITICAL_SECTION {
    DebugInfo: *mut c_void,
    LockCount: i32,
    RecursionCount: i32,
    OwningThread: *mut c_void,
    LockSemaphore: *mut c_void,
    SpinCount: u32,
}

#[link(name = "kernel32")]
extern "system" {
    fn SwitchToThread() -> i32;
    fn Sleep(dwMilliseconds: u32);
    fn InitializeCriticalSection(lpCriticalSection: *mut CRITICAL_SECTION);
    fn EnterCriticalSection(lpCriticalSection: *mut CRITICAL_SECTION);
    fn TryEnterCriticalSection(lpCriticalSection: *mut CRITICAL_SECTION) -> i32;
    fn LeaveCriticalSection(lpCriticalSection: *mut CRITICAL_SECTION);
    fn DeleteCriticalSection(lpCriticalSection: *mut CRITICAL_SECTION);
}

pub fn impl_backoff_snooze(step: &mut u32) {
    *step = step.saturating_add(1);
    let reps = (*step).min(16);
    for _ in 0..reps {
        unsafe {
            let r = SwitchToThread();
            if r == 0 { Sleep(0); }
        }
    }
}

pub unsafe fn impl_create_spinlock() -> *mut c_void {
    let mut cs = Box::new(CRITICAL_SECTION { DebugInfo: std::ptr::null_mut(), LockCount: 0, RecursionCount: 0, OwningThread: std::ptr::null_mut(), LockSemaphore: std::ptr::null_mut(), SpinCount: 0 });
    InitializeCriticalSection(&mut *cs);
    Box::into_raw(cs) as *mut c_void
}

pub unsafe fn impl_destroy_spinlock(h: *mut c_void) {
    if !h.is_null() {
        let cs = h as *mut CRITICAL_SECTION;
        DeleteCriticalSection(cs);
        drop(Box::from_raw(cs));
    }
}

pub unsafe fn impl_try_enter(h: *mut c_void) -> bool { TryEnterCriticalSection(h as *mut CRITICAL_SECTION) != 0 }

pub unsafe fn impl_enter(h: *mut c_void) { EnterCriticalSection(h as *mut CRITICAL_SECTION) }

pub unsafe fn impl_leave(h: *mut c_void) { LeaveCriticalSection(h as *mut CRITICAL_SECTION) }
