use std::ffi::c_void;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Async.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub struct Iocp { pub port: *mut c_void }
    pub fn attach() -> Option<Iocp> { None }
    pub fn register_handle(_iocp: &Iocp, _h: *mut c_void) -> bool { false }
    pub fn get_queued(_iocp: &Iocp, _timeout_ms: u32) -> Option<(*mut c_void, u32, u32)> { None }
}

pub struct Iocp { pub port: *mut c_void }

pub fn attach() -> Option<Iocp> { backend::attach() }
pub fn register_handle(iocp: &Iocp, h: *mut c_void) -> bool { backend::register_handle(iocp, h) }
pub fn get_queued(iocp: &Iocp, timeout_ms: u32) -> Option<(*mut c_void, u32, u32)> { backend::get_queued(iocp, timeout_ms) }

#[repr(C)]
pub struct Overlapped { pub internal: usize, pub internal_high: usize, pub offset: u32, pub offset_high: u32, pub h_event: *mut c_void }

pub fn read_overlapped(h: prm_file::FileHandle, buf: &mut [u8], offset: u64, ov: &mut Overlapped) -> bool { backend::read_overlapped(h.0, buf.as_mut_ptr() as *mut c_void, buf.len() as u32, offset, ov as *mut Overlapped as *mut c_void) }
pub fn write_overlapped(h: prm_file::FileHandle, buf: &[u8], offset: u64, ov: &mut Overlapped) -> bool { backend::write_overlapped(h.0, buf.as_ptr() as *const c_void, buf.len() as u32, offset, ov as *mut Overlapped as *mut c_void) }
