#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;
use crate::{KernelInfo, OSKind, SystemError};

#[repr(C)]
struct SYSTEM_INFO {
    wProcessorArchitecture: u16,
    wReserved: u16,
    dwPageSize: u32,
    lpMinimumApplicationAddress: *mut c_void,
    lpMaximumApplicationAddress: *mut c_void,
    dwActiveProcessorMask: usize,
    dwNumberOfProcessors: u32,
    dwProcessorType: u32,
    dwAllocationGranularity: u32,
    wProcessorLevel: u16,
    wProcessorRevision: u16,
}

#[repr(C)]
struct RTL_OSVERSIONINFOW {
    dwOSVersionInfoSize: u32,
    dwMajorVersion: u32,
    dwMinorVersion: u32,
    dwBuildNumber: u32,
    szCSDVersion: [u16; 128],
}

extern "system" {
    fn GetNativeSystemInfo(lpSystemInfo: *mut SYSTEM_INFO);
    fn GetComputerNameW(lpBuffer: *mut u16, lpnSize: *mut u32) -> i32;
    fn GetUserNameW(lpBuffer: *mut u16, pcbBuffer: *mut u32) -> i32;
}

#[link(name = "ntdll")]
extern "system" {
    fn RtlGetVersion(lpVersionInformation: *mut RTL_OSVERSIONINFOW) -> i32;
}

pub fn impl_kernel() -> KernelInfo {
    let mut v = RTL_OSVERSIONINFOW { dwOSVersionInfoSize: std::mem::size_of::<RTL_OSVERSIONINFOW>() as u32, dwMajorVersion: 0, dwMinorVersion: 0, dwBuildNumber: 0, szCSDVersion: [0; 128] };
    unsafe { RtlGetVersion(&mut v as *mut _); }
    KernelInfo { kind: OSKind::Windows, major: v.dwMajorVersion, minor: v.dwMinorVersion, build: v.dwBuildNumber }
}

pub fn impl_cpu_logical() -> u32 {
    let mut info = SYSTEM_INFO { wProcessorArchitecture: 0, wReserved: 0, dwPageSize: 0, lpMinimumApplicationAddress: std::ptr::null_mut(), lpMaximumApplicationAddress: std::ptr::null_mut(), dwActiveProcessorMask: 0, dwNumberOfProcessors: 0, dwProcessorType: 0, dwAllocationGranularity: 0, wProcessorLevel: 0, wProcessorRevision: 0 };
    unsafe { GetNativeSystemInfo(&mut info as *mut SYSTEM_INFO); }
    info.dwNumberOfProcessors
}

pub fn impl_hostname(buf: &mut [u8]) -> Result<usize, SystemError> {
    let mut wide = [0u16; 256];
    let mut len: u32 = wide.len() as u32;
    let ok = unsafe { GetComputerNameW(wide.as_mut_ptr(), &mut len as *mut u32) };
    if ok == 0 { return Err(SystemError::Unsupported); }
    let s = String::from_utf16_lossy(&wide[..len as usize]);
    fill_buf(buf, s.as_bytes())
}

pub fn impl_username(buf: &mut [u8]) -> Result<usize, SystemError> {
    let mut wide = [0u16; 256];
    let mut len: u32 = wide.len() as u32;
    let ok = unsafe { GetUserNameW(wide.as_mut_ptr(), &mut len as *mut u32) };
    if ok == 0 { return Err(SystemError::Unsupported); }
    let s = String::from_utf16_lossy(&wide[..(len as usize - 1)]);
    fill_buf(buf, s.as_bytes())
}

fn fill_buf(buf: &mut [u8], src: &[u8]) -> Result<usize, SystemError> {
    if buf.len() < src.len() { return Err(SystemError::BufferTooSmall); }
    let n = src.len();
    buf[..n].copy_from_slice(src);
    Ok(n)
}
