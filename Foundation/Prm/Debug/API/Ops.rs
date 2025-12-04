mod types;
pub use types::*;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Debug.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub fn impl_is_debugger_present() -> bool { false }
    pub fn impl_debug_break() {}
    pub fn impl_get_last_error() -> u32 { 0 }
    pub fn impl_format_error_message(_code: u32, _buf: &mut [u8]) -> Result<usize, DebugError> { Err(DebugError::Unsupported) }
    pub fn impl_capture_backtrace(_skip: u32, _frames: &mut [usize]) -> usize { 0 }
    pub fn impl_output_debug_string(_text: &str) {}
    pub fn impl_symbols_init() -> Result<(), DebugError> { Err(DebugError::Unsupported) }
    pub fn impl_symbols_cleanup() {}
    pub fn impl_resolve_symbol(_addr: usize, _buf: &mut [u8]) -> Result<usize, DebugError> { Err(DebugError::Unsupported) }
    pub fn impl_resolve_line(_addr: usize, _file_buf: &mut [u8]) -> Result<(usize, u32), DebugError> { Err(DebugError::Unsupported) }
}

pub fn is_debugger_present() -> bool { backend::impl_is_debugger_present() }
pub fn debug_break() { backend::impl_debug_break() }
pub fn get_last_error() -> u32 { backend::impl_get_last_error() }
pub fn format_error_message(code: u32, buf: &mut [u8]) -> Result<usize, DebugError> { backend::impl_format_error_message(code, buf) }
pub fn capture_backtrace(skip: u32, frames: &mut [usize]) -> usize { backend::impl_capture_backtrace(skip, frames) }
pub fn output_debug_string(text: &str) { backend::impl_output_debug_string(text) }
pub fn symbols_init() -> Result<(), DebugError> { backend::impl_symbols_init() }
pub fn symbols_cleanup() { backend::impl_symbols_cleanup() }
pub fn resolve_symbol(addr: usize, buf: &mut [u8]) -> Result<usize, DebugError> { backend::impl_resolve_symbol(addr, buf) }
pub fn resolve_line(addr: usize, file_buf: &mut [u8]) -> Result<(usize, u32), DebugError> { backend::impl_resolve_line(addr, file_buf) }
