#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;
use crate::*;
use std::ptr;

type HMODULE = *mut c_void;

#[link(name = "kernel32")]
extern "system" {
    fn LoadLibraryW(lpLibFileName: *const u16) -> HMODULE;
    fn FreeLibrary(hLibModule: HMODULE) -> i32;
    fn GetProcAddress(hModule: HMODULE, lpProcName: *const i8) -> *mut c_void;
    fn MultiByteToWideChar(CodePage: u32, dwFlags: u32, lpMultiByteStr: *const i8, cbMultiByte: i32, lpWideCharStr: *mut u16, cchWideChar: i32) -> i32;
}

pub fn impl_load(name: &str) -> Result<LibHandle, DynamicLibraryError> {
    const CP_UTF8: u32 = 65001;
    let src = name.as_bytes();
    let mut len = unsafe { MultiByteToWideChar(CP_UTF8, 0, src.as_ptr() as *const i8, src.len() as i32, ptr::null_mut(), 0) };
    if len <= 0 { return Err(DynamicLibraryError::Failed); }
    len += 1;
    let mut w = vec![0u16; len as usize];
    let wrote = unsafe { MultiByteToWideChar(CP_UTF8, 0, src.as_ptr() as *const i8, src.len() as i32, w.as_mut_ptr(), len - 1) };
    if wrote <= 0 { return Err(DynamicLibraryError::Failed); }
    w[wrote as usize] = 0;
    let h = unsafe { LoadLibraryW(w.as_ptr()) };
    if h.is_null() { Err(DynamicLibraryError::Failed) } else { Ok(LibHandle(h)) }
}

pub fn impl_close(h: LibHandle) { unsafe { FreeLibrary(h.0 as HMODULE); } }

pub fn impl_get_symbol(h: LibHandle, name: &str) -> Result<*mut c_void, DynamicLibraryError> {
    let bytes = name.as_bytes();
    let mut ansi = Vec::<i8>::with_capacity(bytes.len() + 1);
    for &b in bytes { ansi.push(b as i8); }
    ansi.push(0);
    let p = unsafe { GetProcAddress(h.0 as HMODULE, ansi.as_ptr()) };
    if p.is_null() { Err(DynamicLibraryError::Failed) } else { Ok(p) }
}
