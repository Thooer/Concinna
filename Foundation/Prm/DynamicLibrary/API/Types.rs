use std::ffi::c_void;

#[derive(Clone, Copy)]
pub struct LibHandle(pub *mut c_void);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DynamicLibraryError { Unsupported, Failed }

