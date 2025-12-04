use std::ffi::c_void;

#[derive(Clone, Copy)]
pub struct WindowHandle(pub *mut c_void);

#[derive(Clone, Copy)]
pub struct WindowDesc {
    pub width: u32,
    pub height: u32,
    pub resizable: bool,
    pub visible: bool,
}

impl Default for WindowDesc {
    fn default() -> Self {
        Self { width: 1280, height: 720, resizable: true, visible: true }
    }
}

#[derive(Clone, Copy)]
pub enum CursorMode { Normal, Hidden, Locked }

pub type WndProcCallback = fn(hwnd: *mut c_void, msg: u32, wparam: usize, lparam: isize) -> i64;
pub type RawEventCallback = fn(hwnd: *mut c_void, msg: u32, wparam: usize, lparam: isize);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum WindowError { Unsupported, Failed }

