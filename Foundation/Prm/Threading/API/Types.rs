use std::ffi::c_void;

#[derive(Clone, Copy)]
pub struct ThreadHandle(pub *mut c_void);
#[derive(Clone, Copy)]
pub struct MutexHandle(pub *mut c_void);
#[derive(Clone, Copy)]
pub struct SemaphoreHandle(pub *mut c_void);
#[derive(Clone, Copy)]
pub struct EventHandle(pub *mut c_void);

pub type ThreadFunc = fn(*mut c_void);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ThreadingError { Unsupported, Failed }

