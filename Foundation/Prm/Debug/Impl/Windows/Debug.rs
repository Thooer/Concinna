#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;
use std::ptr;
use crate::*;

#[link(name = "kernel32")]
extern "system" {
    fn IsDebuggerPresent() -> i32;
    fn DebugBreak();
    fn GetLastError() -> u32;
    fn FormatMessageW(dwFlags: u32, lpSource: *const c_void, dwMessageId: u32, dwLanguageId: u32, lpBuffer: *mut u16, nSize: u32, Arguments: *mut c_void) -> u32;
    fn RtlCaptureStackBackTrace(FramesToSkip: u32, FramesToCapture: u32, BackTrace: *mut *mut c_void, BackTraceHash: *mut u32) -> u16;
    fn OutputDebugStringW(lpOutputString: *const u16);
    fn WideCharToMultiByte(CodePage: u32, dwFlags: u32, lpWideCharStr: *const u16, cchWideChar: i32, lpMultiByteStr: *mut i8, cbMultiByte: i32, lpDefaultChar: *const i8, lpUsedDefaultChar: *mut i32) -> i32;
    fn MultiByteToWideChar(CodePage: u32, dwFlags: u32, lpMultiByteStr: *const i8, cbMultiByte: i32, lpWideCharStr: *mut u16, cchWideChar: i32) -> i32;
    fn GetCurrentProcess() -> *mut c_void;
}

#[link(name = "dbghelp")]
extern "system" {
    fn SymInitialize(hProcess: *mut c_void, UserSearchPath: *const u16, fInvadeProcess: i32) -> i32;
    fn SymCleanup(hProcess: *mut c_void) -> i32;
    fn SymSetOptions(Options: u32) -> u32;
    fn SymFromAddrW(hProcess: *mut c_void, Address: u64, Displacement: *mut u64, Symbol: *mut SYMBOL_INFOW) -> i32;
    fn SymGetLineFromAddrW64(hProcess: *mut c_void, qwAddr: u64, pdwDisplacement: *mut u32, Line: *mut IMAGEHLP_LINEW64) -> i32;
}

#[repr(C)]
struct SYMBOL_INFOW {
    SizeOfStruct: u32,
    TypeIndex: u32,
    Reserved: [u64; 2],
    Index: u32,
    Size: u32,
    ModBase: u64,
    Flags: u32,
    Value: u64,
    Address: u64,
    Register: u32,
    Scope: u32,
    Tag: u32,
    NameLen: u32,
    MaxNameLen: u32,
    Name: [u16; 1],
}

#[repr(C)]
struct IMAGEHLP_LINEW64 {
    SizeOfStruct: u32,
    Key: *mut c_void,
    LineNumber: u32,
    FileName: *mut u16,
    Address: u64,
}

pub fn impl_is_debugger_present() -> bool { unsafe { IsDebuggerPresent() != 0 } }

pub fn impl_debug_break() { unsafe { DebugBreak() } }

pub fn impl_get_last_error() -> u32 { unsafe { GetLastError() } }

pub fn impl_format_error_message(code: u32, buf: &mut [u8]) -> Result<usize, DebugError> {
    const FM_FROM_SYSTEM: u32 = 0x00001000;
    const FM_IGNORE_INSERTS: u32 = 0x00000200;
    let mut wide = [0u16; 512];
    let n = unsafe { FormatMessageW(FM_FROM_SYSTEM | FM_IGNORE_INSERTS, ptr::null(), code, 0, wide.as_mut_ptr(), wide.len() as u32, ptr::null_mut()) };
    if n == 0 { return Err(DebugError::Failed); }
    let mut end = n as usize;
    while end > 0 { let c = wide[end - 1]; if c == 0 || c == 10 || c == 13 { end -= 1; } else { break; } }
    const CP_UTF8: u32 = 65001;
    let mut out = vec![0i8; end * 4 + 1];
    let wrote = unsafe { WideCharToMultiByte(CP_UTF8, 0, wide.as_ptr(), end as i32, out.as_mut_ptr(), out.len() as i32, ptr::null(), ptr::null_mut()) };
    if wrote <= 0 { return Err(DebugError::Failed); }
    let bytes = &out[..wrote as usize];
    if buf.len() < bytes.len() { return Err(DebugError::BufferTooSmall); }
    let src = unsafe { std::slice::from_raw_parts(bytes.as_ptr() as *const u8, bytes.len()) };
    buf[..src.len()].copy_from_slice(src);
    Ok(src.len())
}

pub fn impl_capture_backtrace(skip: u32, frames: &mut [usize]) -> usize {
    let mut ptrs = vec![ptr::null_mut::<c_void>(); frames.len()];
    let captured = unsafe { RtlCaptureStackBackTrace(skip, frames.len() as u32, ptrs.as_mut_ptr(), ptr::null_mut()) } as usize;
    for i in 0..captured { frames[i] = ptrs[i] as usize; }
    captured
}

pub fn impl_output_debug_string(text: &str) {
    const CP_UTF8: u32 = 65001;
    let src = text.as_bytes();
    let mut len = unsafe { MultiByteToWideChar(CP_UTF8, 0, src.as_ptr() as *const i8, src.len() as i32, ptr::null_mut(), 0) };
    if len <= 0 { return; }
    len += 1;
    let mut w = vec![0u16; len as usize];
    let wrote = unsafe { MultiByteToWideChar(CP_UTF8, 0, src.as_ptr() as *const i8, src.len() as i32, w.as_mut_ptr(), len - 1) };
    if wrote <= 0 { return; }
    w[wrote as usize] = 0;
    unsafe { OutputDebugStringW(w.as_ptr()) };
}

pub fn impl_symbols_init() -> Result<(), DebugError> {
    let proc = unsafe { GetCurrentProcess() };
    let opts = 0x00000002u32 | 0x00000010u32;
    unsafe { SymSetOptions(opts); }
    let ok = unsafe { SymInitialize(proc, ptr::null(), 1) };
    if ok == 0 { Err(DebugError::Failed) } else { Ok(()) }
}

pub fn impl_symbols_cleanup() {
    let proc = unsafe { GetCurrentProcess() };
    unsafe { SymCleanup(proc); }
}

pub fn impl_resolve_symbol(addr: usize, buf: &mut [u8]) -> Result<usize, DebugError> {
    let proc = unsafe { GetCurrentProcess() };
    let name_cap = 512usize;
    let base = std::mem::size_of::<SYMBOL_INFOW>();
    let size = base + (name_cap - 1) * 2;
    let mut raw = vec![0u8; size];
    let p = raw.as_mut_ptr() as *mut SYMBOL_INFOW;
    unsafe {
        (*p).SizeOfStruct = std::mem::size_of::<SYMBOL_INFOW>() as u32;
        (*p).MaxNameLen = name_cap as u32;
        let mut disp = 0u64;
        let ok = SymFromAddrW(proc, addr as u64, &mut disp as *mut u64, p);
        if ok == 0 { return Err(DebugError::Failed); }
        let wptr = (*p).Name.as_ptr();
        let wlen = (*p).NameLen as i32;
        const CP_UTF8: u32 = 65001;
        let mut out = vec![0i8; (wlen as usize) * 4 + 1];
        let wrote = WideCharToMultiByte(CP_UTF8, 0, wptr, wlen, out.as_mut_ptr(), out.len() as i32, ptr::null(), ptr::null_mut());
        if wrote <= 0 { return Err(DebugError::Failed); }
        let bytes = &out[..wrote as usize];
        if buf.len() < bytes.len() { return Err(DebugError::BufferTooSmall); }
        let src = std::slice::from_raw_parts(bytes.as_ptr() as *const u8, bytes.len());
        buf[..src.len()].copy_from_slice(src);
        Ok(src.len())
    }
}

pub fn impl_resolve_line(addr: usize, file_buf: &mut [u8]) -> Result<(usize, u32), DebugError> {
    let proc = unsafe { GetCurrentProcess() };
    let mut line = IMAGEHLP_LINEW64 { SizeOfStruct: std::mem::size_of::<IMAGEHLP_LINEW64>() as u32, Key: ptr::null_mut(), LineNumber: 0, FileName: ptr::null_mut(), Address: 0 };
    let mut disp = 0u32;
    let ok = unsafe { SymGetLineFromAddrW64(proc, addr as u64, &mut disp as *mut u32, &mut line as *mut IMAGEHLP_LINEW64) };
    if ok == 0 { return Err(DebugError::Failed); }
    let mut len = 0usize;
    unsafe { while *line.FileName.add(len) != 0 { len += 1; } }
    const CP_UTF8: u32 = 65001;
    let mut out = vec![0i8; len * 4 + 1];
    let wrote = unsafe { WideCharToMultiByte(CP_UTF8, 0, line.FileName, len as i32, out.as_mut_ptr(), out.len() as i32, ptr::null(), ptr::null_mut()) };
    if wrote <= 0 { return Err(DebugError::Failed); }
    let bytes = &out[..wrote as usize];
    if file_buf.len() < bytes.len() { return Err(DebugError::BufferTooSmall); }
    let src = unsafe { std::slice::from_raw_parts(bytes.as_ptr() as *const u8, bytes.len()) };
    file_buf[..src.len()].copy_from_slice(src);
    Ok((src.len(), line.LineNumber))
}
