mod types;
pub use types::*;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Window.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub fn impl_create(_desc: &WindowDesc, _cb: Option<WndProcCallback>) -> Result<WindowHandle, WindowError> { Err(WindowError::Unsupported) }
    pub fn impl_destroy(_h: WindowHandle) {}
    pub fn impl_show(_h: WindowHandle) -> Result<(), WindowError> { Err(WindowError::Unsupported) }
    pub fn impl_hide(_h: WindowHandle) -> Result<(), WindowError> { Err(WindowError::Unsupported) }
    pub fn impl_set_title(_h: WindowHandle, _title: &str) -> Result<(), WindowError> { Err(WindowError::Unsupported) }
    pub fn impl_resize(_h: WindowHandle, _w: u32, _hgt: u32) -> Result<(), WindowError> { Err(WindowError::Unsupported) }
    pub fn impl_set_cursor_mode(_h: WindowHandle, _m: CursorMode) -> Result<(), WindowError> { Err(WindowError::Unsupported) }
    pub fn impl_process_one_message(_h: Option<WindowHandle>) -> bool { false }
    pub fn impl_set_raw_event_callback(_cb: Option<RawEventCallback>) {}
    pub fn impl_native(_h: WindowHandle) -> Result<*mut std::ffi::c_void, WindowError> { Err(WindowError::Unsupported) }
}

pub fn create(desc: &WindowDesc, cb: Option<WndProcCallback>) -> Result<WindowHandle, WindowError> { backend::impl_create(desc, cb) }
pub fn destroy(h: WindowHandle) { backend::impl_destroy(h) }
pub fn show(h: WindowHandle) -> Result<(), WindowError> { backend::impl_show(h) }
pub fn hide(h: WindowHandle) -> Result<(), WindowError> { backend::impl_hide(h) }
pub fn set_title(h: WindowHandle, title: &str) -> Result<(), WindowError> { backend::impl_set_title(h, title) }
pub fn resize(h: WindowHandle, w: u32, hgt: u32) -> Result<(), WindowError> { backend::impl_resize(h, w, hgt) }
pub fn set_cursor_mode(h: WindowHandle, m: CursorMode) -> Result<(), WindowError> { backend::impl_set_cursor_mode(h, m) }
pub fn process_one_message(h: Option<WindowHandle>) -> bool { backend::impl_process_one_message(h) }
pub fn set_raw_event_callback(cb: Option<RawEventCallback>) { backend::impl_set_raw_event_callback(cb) }
pub fn native(h: WindowHandle) -> Result<*mut std::ffi::c_void, WindowError> { backend::impl_native(h) }
