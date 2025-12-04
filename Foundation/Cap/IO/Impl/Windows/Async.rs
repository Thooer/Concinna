use std::ffi::c_void;
use crate::async_io::Iocp;
use crate::async_io::Overlapped;

#[link(name = "kernel32")]
extern "system" {
    fn CreateIoCompletionPort(file_handle: *mut c_void, existing_port: *mut c_void, key: u32, concurrency: u32) -> *mut c_void;
    fn GetQueuedCompletionStatus(port: *mut c_void, bytes: *mut u32, key: *mut u32, overlapped: *mut *mut c_void, timeout_ms: u32) -> i32;
    fn ReadFile(hFile: *mut c_void, lpBuffer: *mut c_void, nNumberOfBytesToRead: u32, lpNumberOfBytesRead: *mut u32, lpOverlapped: *mut c_void) -> i32;
    fn WriteFile(hFile: *mut c_void, lpBuffer: *const c_void, nNumberOfBytesToWrite: u32, lpNumberOfBytesWritten: *mut u32, lpOverlapped: *mut c_void) -> i32;
}

pub fn attach() -> Option<Iocp> {
    let p = unsafe { CreateIoCompletionPort(!0usize as *mut c_void, std::ptr::null_mut(), 0, 0) };
    if p.is_null() { None } else { Some(Iocp { port: p }) }
}

pub fn register_handle(iocp: &Iocp, h: *mut c_void) -> bool {
    let p = unsafe { CreateIoCompletionPort(h, iocp.port, 0, 0) };
    !p.is_null()
}

pub fn get_queued(iocp: &Iocp, timeout_ms: u32) -> Option<(*mut c_void, u32, u32)> {
    let mut bytes: u32 = 0; let mut key: u32 = 0; let mut ov: *mut c_void = std::ptr::null_mut();
    let ok = unsafe { GetQueuedCompletionStatus(iocp.port, &mut bytes as *mut u32, &mut key as *mut u32, &mut ov as *mut *mut c_void, timeout_ms) };
    if ok != 0 && !ov.is_null() { Some((ov, bytes, key)) } else { None }
}

pub fn read_overlapped(h: *mut c_void, buf: *mut c_void, len: u32, offset: u64, ov: *mut c_void) -> bool {
    unsafe {
        let ovp = ov as *mut Overlapped;
        (*ovp).offset = (offset & 0xFFFF_FFFF) as u32;
        (*ovp).offset_high = (offset >> 32) as u32;
        let mut n: u32 = 0;
        let ok = ReadFile(h, buf, len, &mut n as *mut u32, ov);
        ok != 0
    }
}

pub fn write_overlapped(h: *mut c_void, buf: *const c_void, len: u32, offset: u64, ov: *mut c_void) -> bool {
    unsafe {
        let ovp = ov as *mut Overlapped;
        (*ovp).offset = (offset & 0xFFFF_FFFF) as u32;
        (*ovp).offset_high = (offset >> 32) as u32;
        let mut n: u32 = 0;
        let ok = WriteFile(h, buf, len, &mut n as *mut u32, ov);
        ok != 0
    }
}
