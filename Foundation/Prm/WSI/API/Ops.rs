mod types;
pub use types::*;
use prm_window::WindowHandle;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/WSI.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub fn impl_enumerate_adapters() -> Result<Vec<AdapterInfo>, WSIError> { Err(WSIError::Unsupported) }
    pub fn impl_create_cpu_present(_hwnd: WindowHandle, _width: u32, _height: u32) -> Result<CpuPresentHandle, WSIError> { Err(WSIError::Unsupported) }
    pub fn impl_destroy_cpu_present(_state: CpuPresentHandle) -> Result<(), WSIError> { Err(WSIError::Unsupported) }
    pub fn impl_cpu_get_buffer(_state: CpuPresentHandle) -> Result<*mut u8, WSIError> { Err(WSIError::Unsupported) }
    pub fn impl_cpu_get_pitch(_state: CpuPresentHandle) -> Result<u32, WSIError> { Err(WSIError::Unsupported) }
    pub fn impl_cpu_present(_state: CpuPresentHandle) -> Result<(), WSIError> { Err(WSIError::Unsupported) }
    pub fn impl_create_vulkan_surface(_instance: *mut std::ffi::c_void, _hwnd: WindowHandle) -> Result<VulkanSurface, WSIError> { Err(WSIError::Unsupported) }
    pub fn impl_destroy_vulkan_surface(_instance: *mut std::ffi::c_void, _surface: VulkanSurface) -> Result<(), WSIError> { Err(WSIError::Unsupported) }
}

pub fn enumerate_adapters() -> Result<Vec<AdapterInfo>, WSIError> { backend::impl_enumerate_adapters() }
pub fn create_cpu_present(hwnd: WindowHandle, width: u32, height: u32) -> Result<CpuPresentHandle, WSIError> { backend::impl_create_cpu_present(hwnd, width, height) }
pub fn destroy_cpu_present(state: CpuPresentHandle) -> Result<(), WSIError> { backend::impl_destroy_cpu_present(state) }
pub fn cpu_get_buffer(state: CpuPresentHandle) -> Result<*mut u8, WSIError> { backend::impl_cpu_get_buffer(state) }
pub fn cpu_get_pitch(state: CpuPresentHandle) -> Result<u32, WSIError> { backend::impl_cpu_get_pitch(state) }
pub fn cpu_present(state: CpuPresentHandle) -> Result<(), WSIError> { backend::impl_cpu_present(state) }
pub fn create_vulkan_surface(instance: *mut std::ffi::c_void, hwnd: WindowHandle) -> Result<VulkanSurface, WSIError> { backend::impl_create_vulkan_surface(instance, hwnd) }
pub fn destroy_vulkan_surface(instance: *mut std::ffi::c_void, surface: VulkanSurface) -> Result<(), WSIError> { backend::impl_destroy_vulkan_surface(instance, surface) }
