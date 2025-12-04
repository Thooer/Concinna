#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;
use crate::*;

#[repr(C)]
struct SYSTEM_INFO_MIN {
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

extern "system" {
    fn VirtualAlloc(lpAddress: *mut c_void, dwSize: usize, flAllocationType: u32, flProtect: u32) -> *mut c_void;
    fn VirtualFree(lpAddress: *mut c_void, dwSize: usize, dwFreeType: u32) -> i32;
    fn VirtualProtect(lpAddress: *mut c_void, dwSize: usize, flNewProtect: u32, lpflOldProtect: *mut u32) -> i32;
    fn GetSystemInfo(lpSystemInfo: *mut SYSTEM_INFO_MIN);
    fn GetLargePageMinimum() -> usize;
    fn GetCurrentProcess() -> *mut c_void;
    fn VirtualAllocExNuma(hProcess: *mut c_void, lpAddress: *mut c_void, dwSize: usize, flAllocationType: u32, flProtect: u32, nndPreferred: u32) -> *mut c_void;
    fn HeapCreate(flOptions: u32, dwInitialSize: usize, dwMaximumSize: usize) -> *mut c_void;
    fn HeapDestroy(hHeap: *mut c_void) -> i32;
    fn GetProcessHeap() -> *mut c_void;
    fn HeapAlloc(hHeap: *mut c_void, dwFlags: u32, dwBytes: usize) -> *mut c_void;
    fn HeapFree(hHeap: *mut c_void, dwFlags: u32, lpMem: *mut c_void) -> i32;
}

const MEM_COMMIT: u32 = 0x0000_1000;
const MEM_RESERVE: u32 = 0x0000_2000;
const MEM_DECOMMIT: u32 = 0x0000_4000;
const MEM_RELEASE: u32 = 0x0000_8000;
const MEM_LARGE_PAGES: u32 = 0x2000_0000;
const PAGE_NOACCESS: u32 = 0x0000_0001;
const PAGE_READONLY: u32 = 0x0000_0002;
const PAGE_READWRITE: u32 = 0x0000_0004;
const PAGE_EXECUTE_READ: u32 = 0x0000_0020;
const PAGE_EXECUTE_READWRITE: u32 = 0x0000_0040;

fn win_protect(p: PageProtection) -> u32 {
    match p { PageProtection::NoAccess => PAGE_NOACCESS, PageProtection::ReadOnly => PAGE_READONLY, PageProtection::ReadWrite => PAGE_READWRITE, PageProtection::ExecuteRead => PAGE_EXECUTE_READ, PageProtection::ExecuteReadWrite => PAGE_EXECUTE_READWRITE }
}

pub fn vm_reserve(size: usize) -> Result<*mut c_void, MemoryError> {
    let p = unsafe { VirtualAlloc(std::ptr::null_mut(), size, MEM_RESERVE, PAGE_NOACCESS) };
    if p.is_null() { Err(MemoryError::ReserveFailed) } else { Ok(p) }
}

pub fn vm_commit(base: *mut c_void, size: usize, protection: PageProtection) -> Result<(), MemoryError> {
    if base.is_null() || size == 0 { return Err(MemoryError::InvalidArgument); }
    let prot = win_protect(protection);
    let p = unsafe { VirtualAlloc(base, size, MEM_COMMIT, prot) };
    if p.is_null() { Err(MemoryError::CommitFailed) } else { Ok(()) }
}

pub fn vm_protect(base: *mut c_void, size: usize, protection: PageProtection) -> Result<(), MemoryError> {
    if base.is_null() || size == 0 { return Err(MemoryError::InvalidArgument); }
    let mut old: u32 = 0;
    let prot = win_protect(protection);
    let ok = unsafe { VirtualProtect(base, size, prot, &mut old as *mut u32) };
    if ok == 0 { Err(MemoryError::ProtectFailed) } else { Ok(()) }
}

pub fn vm_decommit(base: *mut c_void, size: usize) -> Result<(), MemoryError> {
    if base.is_null() || size == 0 { return Err(MemoryError::InvalidArgument); }
    let ok = unsafe { VirtualFree(base, size, MEM_DECOMMIT) };
    if ok == 0 { Err(MemoryError::DecommitFailed) } else { Ok(()) }
}

pub fn vm_release(base: *mut c_void) -> Result<(), MemoryError> {
    if base.is_null() { return Err(MemoryError::InvalidArgument); }
    let ok = unsafe { VirtualFree(base, 0, MEM_RELEASE) };
    if ok == 0 { Err(MemoryError::ReleaseFailed) } else { Ok(()) }
}

fn query_page_size() -> usize { let mut si = SYSTEM_INFO_MIN { wProcessorArchitecture: 0, wReserved: 0, dwPageSize: 0, lpMinimumApplicationAddress: std::ptr::null_mut(), lpMaximumApplicationAddress: std::ptr::null_mut(), dwActiveProcessorMask: 0, dwNumberOfProcessors: 0, dwProcessorType: 0, dwAllocationGranularity: 0, wProcessorLevel: 0, wProcessorRevision: 0 }; unsafe { GetSystemInfo(&mut si as *mut _) }; let ps = si.dwPageSize; if ps == 0 { 4096 } else { ps as usize } }
fn query_alloc_granularity() -> usize { let mut si = SYSTEM_INFO_MIN { wProcessorArchitecture: 0, wReserved: 0, dwPageSize: 0, lpMinimumApplicationAddress: std::ptr::null_mut(), lpMaximumApplicationAddress: std::ptr::null_mut(), dwActiveProcessorMask: 0, dwNumberOfProcessors: 0, dwProcessorType: 0, dwAllocationGranularity: 0, wProcessorLevel: 0, wProcessorRevision: 0 }; unsafe { GetSystemInfo(&mut si as *mut _) }; let g = si.dwAllocationGranularity; if g == 0 { 64 * 1024 } else { g as usize } }

pub fn vm_page_size() -> usize { query_page_size() }
pub fn vm_alloc_granularity() -> usize { query_alloc_granularity() }

pub fn vm_large_page_size() -> Result<usize, MemoryError> { let lp = unsafe { GetLargePageMinimum() }; if lp == 0 { Err(MemoryError::LargePagesNotSupported) } else { Ok(lp as usize) } }

pub fn vm_reserve_ex(size: usize, numa_node_id: u32, use_large_pages: bool) -> Result<*mut c_void, MemoryError> {
    let process = unsafe { GetCurrentProcess() };
    if process.is_null() { return Err(MemoryError::QueryFailed); }
    let mut alloc_flags = MEM_RESERVE;
    let mut prot = PAGE_NOACCESS;
    let mut request_size = size;
    if use_large_pages {
        let lp = unsafe { GetLargePageMinimum() };
        if lp == 0 { return Err(MemoryError::LargePagesNotSupported); }
        alloc_flags |= MEM_COMMIT | MEM_LARGE_PAGES;
        prot = PAGE_READWRITE;
        let mask = lp - 1;
        request_size = (request_size + mask) & !mask;
    }
    let p = unsafe { VirtualAllocExNuma(process, std::ptr::null_mut(), request_size, alloc_flags, prot, numa_node_id) };
    if p.is_null() { Err(MemoryError::ReserveFailed) } else { Ok(p) }
}

pub fn heap_create() -> Result<HeapHandle, MemoryError> { let h = unsafe { HeapCreate(0, 0, 0) }; if h.is_null() { Err(MemoryError::HeapCreateFailed) } else { Ok(HeapHandle(h)) } }
pub fn heap_destroy(h: HeapHandle) -> Result<(), MemoryError> { if h.0.is_null() { return Err(MemoryError::InvalidArgument); } let ok = unsafe { HeapDestroy(h.0) }; if ok == 0 { Err(MemoryError::HeapDestroyFailed) } else { Ok(()) } }
pub fn heap_process_default() -> HeapHandle { HeapHandle(unsafe { GetProcessHeap() }) }
pub fn heap_alloc_raw(h: HeapHandle, size: usize) -> Result<*mut c_void, MemoryError> { if h.0.is_null() || size == 0 { return Err(MemoryError::InvalidArgument); } let p = unsafe { HeapAlloc(h.0, 0, size) }; if p.is_null() { Err(MemoryError::HeapAllocFailed) } else { Ok(p) } }
pub fn heap_free_raw(h: HeapHandle, p: *mut c_void) -> Result<(), MemoryError> { if h.0.is_null() || p.is_null() { return Err(MemoryError::InvalidArgument); } let ok = unsafe { HeapFree(h.0, 0, p) }; if ok == 0 { Err(MemoryError::HeapFreeFailed) } else { Ok(()) } }

pub fn heap_alloc(h: HeapHandle, size: usize, alignment: usize) -> Result<*mut c_void, MemoryError> {
    if h.0.is_null() || size == 0 { return Err(MemoryError::InvalidArgument); }
    if alignment == 0 || (alignment & (alignment - 1)) != 0 { return Err(MemoryError::AlignmentNotPowerOfTwo); }
    if alignment > heap_max_alignment() { return Err(MemoryError::AlignmentTooLarge); }
    let extra = alignment - 1 + std::mem::size_of::<*mut c_void>();
    let req = size.checked_add(extra).ok_or(MemoryError::OutOfMemory)?;
    let raw = unsafe { HeapAlloc(h.0, 0, req) };
    if raw.is_null() { return Err(MemoryError::HeapAllocFailed); }
    let base = (raw as usize) + std::mem::size_of::<*mut c_void>();
    let aligned = (base + (alignment - 1)) & !(alignment - 1);
    let marker = aligned as *mut *mut c_void;
    unsafe { std::ptr::write(marker.offset(-1), raw); }
    Ok(aligned as *mut c_void)
}

pub fn heap_free(h: HeapHandle, p: *mut c_void) -> Result<(), MemoryError> {
    if h.0.is_null() || p.is_null() { return Err(MemoryError::InvalidArgument); }
    let marker = p as *mut *mut c_void;
    let raw = unsafe { *marker.offset(-1) };
    let ok = unsafe { HeapFree(h.0, 0, raw) };
    if ok == 0 { Err(MemoryError::HeapFreeFailed) } else { Ok(()) }
}

pub fn heap_max_alignment() -> usize { vm_alloc_granularity() }
