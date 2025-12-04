use std::ffi::c_void;

#[derive(Clone, Copy)]
pub struct AdapterHandle(pub *mut c_void);

#[derive(Clone, Copy)]
pub struct CpuPresentHandle(pub *mut c_void);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum WSIError { Failed, Unsupported }

#[derive(Clone)]
pub struct AdapterInfo {
    pub name: String,
    pub vendor_id: u32,
    pub device_id: u32,
    pub dedicated_video_memory: u64,
    pub flags: u32,
}

#[derive(Clone, Copy)]
pub struct VulkanSurface(pub *mut c_void);
