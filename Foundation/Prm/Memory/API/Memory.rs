use std::ffi::c_void;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MemoryError { InvalidArgument, ReserveFailed, CommitFailed, ProtectFailed, DecommitFailed, ReleaseFailed, HeapCreateFailed, HeapDestroyFailed, HeapAllocFailed, HeapFreeFailed, AlignmentNotPowerOfTwo, AlignmentTooLarge, SizeNotAligned, LargePagesNotSupported, LargePagesNotEnabled, NumaUnavailable, QueryFailed, AccessDenied, OutOfMemory }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PageProtection { NoAccess, ReadOnly, ReadWrite, ExecuteRead, ExecuteReadWrite }

#[derive(Clone, Copy)]
pub struct HeapHandle(pub *mut c_void);

pub fn align_up(value: usize, align: usize) -> usize { if align > 0 { (value + (align - 1)) & !(align - 1) } else { value } }

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Memory.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub fn vm_reserve(_size: usize) -> Result<*mut c_void, MemoryError> { Err(MemoryError::ReserveFailed) }
    pub fn vm_commit(_base: *mut c_void, _size: usize, _prot: PageProtection) -> Result<(), MemoryError> { Err(MemoryError::CommitFailed) }
    pub fn vm_protect(_base: *mut c_void, _size: usize, _prot: PageProtection) -> Result<(), MemoryError> { Err(MemoryError::ProtectFailed) }
    pub fn vm_decommit(_base: *mut c_void, _size: usize) -> Result<(), MemoryError> { Err(MemoryError::DecommitFailed) }
    pub fn vm_release(_base: *mut c_void) -> Result<(), MemoryError> { Err(MemoryError::ReleaseFailed) }
    pub fn vm_page_size() -> usize { 4096 }
    pub fn vm_alloc_granularity() -> usize { 64 * 1024 }
    pub fn vm_large_page_size() -> Result<usize, MemoryError> { Err(MemoryError::LargePagesNotSupported) }
    pub fn vm_reserve_ex(_size: usize, _numa: u32, _lp: bool) -> Result<*mut c_void, MemoryError> { Err(MemoryError::ReserveFailed) }
    pub fn heap_create() -> Result<HeapHandle, MemoryError> { Err(MemoryError::HeapCreateFailed) }
    pub fn heap_destroy(_h: HeapHandle) -> Result<(), MemoryError> { Err(MemoryError::HeapDestroyFailed) }
    pub fn heap_process_default() -> HeapHandle { HeapHandle(std::ptr::null_mut()) }
    pub fn heap_alloc_raw(_h: HeapHandle, _size: usize) -> Result<*mut c_void, MemoryError> { Err(MemoryError::HeapAllocFailed) }
    pub fn heap_free_raw(_h: HeapHandle, _p: *mut c_void) -> Result<(), MemoryError> { Err(MemoryError::HeapFreeFailed) }
    pub fn heap_alloc(_h: HeapHandle, _size: usize, _alignment: usize) -> Result<*mut c_void, MemoryError> { Err(MemoryError::HeapAllocFailed) }
    pub fn heap_free(_h: HeapHandle, _p: *mut c_void) -> Result<(), MemoryError> { Err(MemoryError::HeapFreeFailed) }
    pub fn heap_max_alignment() -> usize { 64 * 1024 }
}

pub fn reserve(size: usize) -> Result<*mut c_void, MemoryError> { backend::vm_reserve(size) }
pub fn commit(base: *mut c_void, size: usize, protection: PageProtection) -> Result<(), MemoryError> { backend::vm_commit(base, size, protection) }
pub fn protect(base: *mut c_void, size: usize, protection: PageProtection) -> Result<(), MemoryError> { backend::vm_protect(base, size, protection) }
pub fn decommit(base: *mut c_void, size: usize) -> Result<(), MemoryError> { backend::vm_decommit(base, size) }
pub fn release(base: *mut c_void) -> Result<(), MemoryError> { backend::vm_release(base) }
pub fn page_size() -> usize { backend::vm_page_size() }
pub fn allocation_granularity() -> usize { backend::vm_alloc_granularity() }
pub fn large_page_size() -> Result<usize, MemoryError> { backend::vm_large_page_size() }
pub fn reserve_ex(size: usize, numa_node_id: u32, use_large_pages: bool) -> Result<*mut c_void, MemoryError> { backend::vm_reserve_ex(size, numa_node_id, use_large_pages) }

pub fn heap_create() -> Result<HeapHandle, MemoryError> { backend::heap_create() }
pub fn heap_destroy(h: HeapHandle) -> Result<(), MemoryError> { backend::heap_destroy(h) }
pub fn heap_process_default() -> HeapHandle { backend::heap_process_default() }
pub fn heap_alloc_raw(h: HeapHandle, size: usize) -> Result<*mut c_void, MemoryError> { backend::heap_alloc_raw(h, size) }
pub fn heap_free_raw(h: HeapHandle, p: *mut c_void) -> Result<(), MemoryError> { backend::heap_free_raw(h, p) }
pub fn heap_alloc(h: HeapHandle, size: usize, alignment: usize) -> Result<*mut c_void, MemoryError> { backend::heap_alloc(h, size, alignment) }
pub fn heap_free(h: HeapHandle, p: *mut c_void) -> Result<(), MemoryError> { backend::heap_free(h, p) }
pub fn heap_max_alignment() -> usize { backend::heap_max_alignment() }

