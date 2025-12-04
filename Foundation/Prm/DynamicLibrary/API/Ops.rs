mod types;
pub use types::*;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/DynamicLibrary.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub fn impl_load(_name: &str) -> Result<LibHandle, DynamicLibraryError> { Err(DynamicLibraryError::Unsupported) }
    pub fn impl_close(_h: LibHandle) {}
    pub fn impl_get_symbol(_h: LibHandle, _name: &str) -> Result<*mut std::ffi::c_void, DynamicLibraryError> { Err(DynamicLibraryError::Unsupported) }
}

pub fn load(name: &str) -> Result<LibHandle, DynamicLibraryError> { backend::impl_load(name) }
pub fn close(h: LibHandle) { backend::impl_close(h) }
pub fn get_symbol(h: LibHandle, name: &str) -> Result<*mut std::ffi::c_void, DynamicLibraryError> { backend::impl_get_symbol(h, name) }

