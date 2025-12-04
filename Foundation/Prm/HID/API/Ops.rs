mod types;
pub use types::*;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/HID.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub fn impl_register_devices(_hwnd: RawWindowHandle) -> Result<(), HidError> { Err(HidError::Unsupported) }
    pub fn impl_on_raw_event(_hwnd: RawWindowHandle, _msg: u32, _wparam: usize, _lparam: isize) {}
    pub fn impl_take_events(_out: &mut [HidEvent]) -> usize { 0 }
}

pub fn register_devices(hwnd: RawWindowHandle) -> Result<(), HidError> { backend::impl_register_devices(hwnd) }
pub fn on_raw_event(hwnd: RawWindowHandle, msg: u32, wparam: usize, lparam: isize) { backend::impl_on_raw_event(hwnd, msg, wparam, lparam) }
pub fn take_events(out: &mut [HidEvent]) -> usize { backend::impl_take_events(out) }

