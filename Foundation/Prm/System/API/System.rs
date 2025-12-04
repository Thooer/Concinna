#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SystemError {
    Unsupported,
    BufferTooSmall,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum OSKind { Unknown, Windows, Linux, MacOS }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct KernelInfo { pub kind: OSKind, pub major: u32, pub minor: u32, pub build: u32 }

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/System.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::{KernelInfo, OSKind, SystemError};
    use std::thread;

    pub fn impl_kernel() -> KernelInfo { KernelInfo { kind: OSKind::Unknown, major: 0, minor: 0, build: 0 } }
    pub fn impl_cpu_logical() -> u32 { thread::available_parallelism().map(|n| n.get() as u32).unwrap_or(1) }
    pub fn impl_hostname(_buf: &mut [u8]) -> Result<usize, SystemError> { Err(SystemError::Unsupported) }
    pub fn impl_username(_buf: &mut [u8]) -> Result<usize, SystemError> { Err(SystemError::Unsupported) }
}

pub fn kernel() -> KernelInfo { backend::impl_kernel() }

pub fn cpu_logical() -> u32 { backend::impl_cpu_logical() }

pub fn hostname(buf: &mut [u8]) -> Result<usize, SystemError> { backend::impl_hostname(buf) }

pub fn username(buf: &mut [u8]) -> Result<usize, SystemError> { backend::impl_username(buf) }
#[path = "Topology.rs"]
pub mod topology;
pub use topology::*;
