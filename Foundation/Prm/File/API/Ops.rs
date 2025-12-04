mod types;
pub use types::*;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/File.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub fn impl_open(_path: &str, _mode: FileOpenMode, _share: FileShareMode) -> Result<FileHandle, FileError> { Err(FileError::Unsupported) }
    pub fn impl_close(_h: FileHandle) -> Result<(), FileError> { Err(FileError::Unsupported) }
    pub fn impl_read(_h: FileHandle, _buf: &mut [u8]) -> Result<usize, FileError> { Err(FileError::Unsupported) }
    pub fn impl_write(_h: FileHandle, _buf: &[u8]) -> Result<usize, FileError> { Err(FileError::Unsupported) }
    pub fn impl_size(_h: FileHandle) -> Result<u64, FileError> { Err(FileError::Unsupported) }
    pub fn impl_seek(_h: FileHandle, _offset: i64, _origin: SeekOrigin) -> Result<u64, FileError> { Err(FileError::Unsupported) }
    pub fn impl_flush(_h: FileHandle) -> Result<(), FileError> { Err(FileError::Unsupported) }
    pub fn impl_map(_h: FileHandle, _offset: u64, _size: usize, _access: MapAccess) -> Result<Mapping, FileError> { Err(FileError::Unsupported) }
    pub fn impl_unmap(_m: &Mapping) -> Result<(), FileError> { Err(FileError::Unsupported) }
    pub fn impl_flush_mapped(_addr: *mut std::ffi::c_void, _size: usize) -> Result<(), FileError> { Err(FileError::Unsupported) }
    pub fn impl_stdout_handle() -> FileHandle { FileHandle(std::ptr::null_mut()) }
    pub fn impl_stderr_handle() -> FileHandle { FileHandle(std::ptr::null_mut()) }
}

pub fn open(path: &str, mode: FileOpenMode, share: FileShareMode) -> Result<FileHandle, FileError> { backend::impl_open(path, mode, share) }
pub fn open_overlapped(path: &str, mode: FileOpenMode, share: FileShareMode) -> Result<FileHandle, FileError> { backend::impl_open_overlapped(path, mode, share) }
pub fn close(h: FileHandle) -> Result<(), FileError> { backend::impl_close(h) }
pub fn read(h: FileHandle, buf: &mut [u8]) -> Result<usize, FileError> { backend::impl_read(h, buf) }
pub fn write(h: FileHandle, buf: &[u8]) -> Result<usize, FileError> { backend::impl_write(h, buf) }
pub fn size(h: FileHandle) -> Result<u64, FileError> { backend::impl_size(h) }
pub fn seek(h: FileHandle, offset: i64, origin: SeekOrigin) -> Result<u64, FileError> { backend::impl_seek(h, offset, origin) }
pub fn flush(h: FileHandle) -> Result<(), FileError> { backend::impl_flush(h) }
pub fn map(h: FileHandle, offset: u64, size: usize, access: MapAccess) -> Result<Mapping, FileError> { backend::impl_map(h, offset, size, access) }
pub fn unmap(m: &Mapping) -> Result<(), FileError> { backend::impl_unmap(m) }
pub fn flush_mapped(addr: *mut std::ffi::c_void, size: usize) -> Result<(), FileError> { backend::impl_flush_mapped(addr, size) }
pub fn stdout_handle() -> FileHandle { backend::impl_stdout_handle() }
pub fn stderr_handle() -> FileHandle { backend::impl_stderr_handle() }

