use std::ffi::c_void;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum HidError { Unsupported, Failed }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum HidEvent {
    KeyDown(u16),
    KeyUp(u16),
    MouseMove(i32, i32),
    MouseLeftDown,
    MouseLeftUp,
    MouseRightDown,
    MouseRightUp,
    MouseMiddleDown,
    MouseMiddleUp,
    MouseWheel(i16),
}

pub type RawWindowHandle = *mut c_void;

