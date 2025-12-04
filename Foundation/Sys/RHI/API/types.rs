use std::ffi::c_void;

#[derive(Clone, Copy)]
pub struct DeviceHandle(pub *mut c_void);

#[derive(Clone, Copy)]
pub struct SwapchainHandle(pub *mut c_void);

#[derive(Clone, Copy)]
pub struct QueueHandle(pub *mut c_void);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RhiError { Failed, Unsupported }

#[derive(Clone, Copy)]
pub struct RhiCtx(pub *mut c_void);
