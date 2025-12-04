#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
type HANDLE = *mut c_void;
use std::ffi::c_void;
use std::sync::OnceLock;
use std::{ptr};

extern "system" {
    fn QueryPerformanceCounter(lpPerformanceCount: *mut i64) -> i32;
    fn QueryPerformanceFrequency(lpFrequency: *mut i64) -> i32;
    fn Sleep(dwMilliseconds: u32);
    fn CreateWaitableTimerW(lpTimerAttributes: *mut c_void, bManualReset: i32, lpTimerName: *const u16) -> HANDLE;
    fn SetWaitableTimer(hTimer: HANDLE, pDueTime: *const i64, lPeriod: i32, pfnCompletionRoutine: *mut c_void, lpArgToCompletionRoutine: *mut c_void, fResume: i32) -> i32;
    fn CloseHandle(hObject: HANDLE) -> i32;
    fn WaitForSingleObject(hHandle: HANDLE, dwMilliseconds: u32) -> u32;
}

static FREQ: OnceLock<i64> = OnceLock::new();

fn qpf() -> i64 {
    *FREQ.get_or_init(|| {
        let mut f: i64 = 0;
        unsafe { QueryPerformanceFrequency(&mut f as *mut i64); }
        if f <= 0 { 1 } else { f }
    })
}

pub fn impl_now_ns() -> i64 {
    let mut c: i64 = 0;
    unsafe { QueryPerformanceCounter(&mut c as *mut i64); }
    let f = qpf();
    c.saturating_mul(1_000_000_000).saturating_div(f)
}

pub fn impl_sleep_ms(ms: u32) {
    unsafe { Sleep(ms) }
}

pub fn impl_sleep_precise_ns(ns: i64) {
    if ns <= 0 { return; }
    let h: HANDLE = unsafe { CreateWaitableTimerW(ptr::null_mut(), 0, ptr::null()) };
    if h.is_null() { unsafe { Sleep((ns / 1_000_000) as u32) }; return; }
    let mut due: i64 = - (ns / 100);
    if due == 0 { due = -1; }
    let ok = unsafe { SetWaitableTimer(h, &due as *const i64, 0, ptr::null_mut(), ptr::null_mut(), 0) };
    if ok == 0 {
        unsafe { CloseHandle(h); Sleep((ns / 1_000_000) as u32) };
        return;
    }
    unsafe { WaitForSingleObject(h, 0xFFFFFFFF); CloseHandle(h); }
}
