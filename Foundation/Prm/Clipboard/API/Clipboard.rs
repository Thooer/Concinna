#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ClipboardError {
    Unsupported,
    BufferTooSmall,
}

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Clipboard.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::ClipboardError;
    pub fn impl_set_text(_text: &str) -> Result<(), ClipboardError> { Err(ClipboardError::Unsupported) }
    pub fn impl_get_text(_buf: &mut [u8]) -> Result<usize, ClipboardError> { Err(ClipboardError::Unsupported) }
}

pub fn set_text(text: &str) -> Result<(), ClipboardError> {
    backend::impl_set_text(text)
}

pub fn get_text(buf: &mut [u8]) -> Result<usize, ClipboardError> {
    backend::impl_get_text(buf)
}

