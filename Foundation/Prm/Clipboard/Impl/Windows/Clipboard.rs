#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;
use std::ptr;
use crate::ClipboardError;

type HANDLE = *mut c_void;

const CF_UNICODETEXT: u32 = 13;
const GMEM_MOVEABLE: u32 = 0x0002;
const GMEM_ZEROINIT: u32 = 0x0040;

#[link(name = "user32")]
extern "system" {
    fn OpenClipboard(hWndNewOwner: HANDLE) -> i32;
    fn CloseClipboard() -> i32;
    fn EmptyClipboard() -> i32;
    fn SetClipboardData(uFormat: u32, hMem: HANDLE) -> HANDLE;
    fn GetClipboardData(uFormat: u32) -> HANDLE;
}

#[link(name = "kernel32")]
extern "system" {
    fn GlobalAlloc(uFlags: u32, dwBytes: usize) -> HANDLE;
    fn GlobalLock(hMem: HANDLE) -> *mut c_void;
    fn GlobalUnlock(hMem: HANDLE) -> i32;
    fn MultiByteToWideChar(CodePage: u32, dwFlags: u32, lpMultiByteStr: *const i8, cbMultiByte: i32, lpWideCharStr: *mut u16, cchWideChar: i32) -> i32;
    fn WideCharToMultiByte(CodePage: u32, dwFlags: u32, lpWideCharStr: *const u16, cchWideChar: i32, lpMultiByteStr: *mut i8, cbMultiByte: i32, lpDefaultChar: *const i8, lpUsedDefaultChar: *mut i32) -> i32;
}

pub fn impl_set_text(text: &str) -> Result<(), ClipboardError> {
    const CP_UTF8: u32 = 65001;
    let src = text.as_bytes();
    let mut len = unsafe { MultiByteToWideChar(CP_UTF8, 0, src.as_ptr() as *const i8, src.len() as i32, std::ptr::null_mut(), 0) };
    if len <= 0 { return Err(ClipboardError::Unsupported); }
    // add NUL terminator
    len += 1;
    let mut wide = vec![0u16; len as usize];
    let wrote = unsafe { MultiByteToWideChar(CP_UTF8, 0, src.as_ptr() as *const i8, src.len() as i32, wide.as_mut_ptr(), len - 1) };
    if wrote <= 0 { return Err(ClipboardError::Unsupported); }
    wide[wrote as usize] = 0;
    unsafe {
        if OpenClipboard(ptr::null_mut()) == 0 { return Err(ClipboardError::Unsupported); }
        if EmptyClipboard() == 0 { CloseClipboard(); return Err(ClipboardError::Unsupported); }
        let bytes = wide.len() * 2;
        let h = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
        if h.is_null() { CloseClipboard(); return Err(ClipboardError::Unsupported); }
        let p = GlobalLock(h) as *mut u16;
        if p.is_null() { CloseClipboard(); return Err(ClipboardError::Unsupported); }
        std::ptr::copy_nonoverlapping(wide.as_ptr(), p, wide.len());
        GlobalUnlock(h);
        if SetClipboardData(CF_UNICODETEXT, h).is_null() { CloseClipboard(); return Err(ClipboardError::Unsupported); }
        CloseClipboard();
    }
    Ok(())
}

pub fn impl_get_text(buf: &mut [u8]) -> Result<usize, ClipboardError> {
    unsafe {
        if OpenClipboard(ptr::null_mut()) == 0 { return Err(ClipboardError::Unsupported); }
        let h = GetClipboardData(CF_UNICODETEXT);
        if h.is_null() { CloseClipboard(); return Err(ClipboardError::Unsupported); }
        let p = GlobalLock(h) as *const u16;
        if p.is_null() { CloseClipboard(); return Err(ClipboardError::Unsupported); }
        let mut len = 0usize;
        while *p.add(len) != 0 { len += 1; }
        let slice = std::slice::from_raw_parts(p, len);
        const CP_UTF8: u32 = 65001;
        let mut out = vec![0i8; len * 4 + 1];
        let wrote = WideCharToMultiByte(CP_UTF8, 0, slice.as_ptr(), len as i32, out.as_mut_ptr(), out.len() as i32, std::ptr::null(), std::ptr::null_mut());
        if wrote <= 0 { GlobalUnlock(h); CloseClipboard(); return Err(ClipboardError::Unsupported); }
        GlobalUnlock(h);
        CloseClipboard();
        let bytes = &out[..wrote as usize];
        if buf.len() < bytes.len() { return Err(ClipboardError::BufferTooSmall); }
        let src = std::slice::from_raw_parts(bytes.as_ptr() as *const u8, bytes.len());
        buf[..src.len()].copy_from_slice(src);
        Ok(src.len())
    }
}
