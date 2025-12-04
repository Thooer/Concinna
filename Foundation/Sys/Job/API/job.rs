use core::ffi::c_void;

#[derive(Clone, Copy)]
pub struct Job { pub func: fn(*mut c_void), pub arg: *mut c_void, pub qos: u8 }

impl Job { pub const fn new(func: fn(*mut c_void), arg: *mut c_void) -> Self { Self { func, arg, qos: 0 } } }
