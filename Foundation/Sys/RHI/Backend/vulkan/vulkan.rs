use std::ffi::c_void;
use prm_window::WindowHandle;
use prm_wsi::{create_vulkan_surface, destroy_vulkan_surface, VulkanSurface};
use crate::RhiError;

#[cfg(target_os = "windows")]
#[path = "../../Impl/Vulkan/ffi.rs"]
mod vkffi;

#[derive(Clone, Copy)]
pub struct VkInstanceHandle(pub *mut c_void);

static APP_NAME: [i8; 9] = [67,111,110,99,105,110,110,97,0];
static ENGINE_NAME: [i8; 9] = [67,111,110,99,105,110,110,97,0];
static EXT_SURFACE: [i8; 16] = [86,75,95,75,72,82,95,115,117,114,102,97,99,101,0,0];
static EXT_WIN32_SURFACE: [i8; 21] = [86,75,95,75,72,82,95,119,105,110,51,50,95,115,117,114,102,97,99,101,0];

pub fn vk_create_instance() -> Result<VkInstanceHandle, RhiError> {
    #[cfg(target_os = "windows")]
    unsafe {
        let ldr = match vkffi::load() { Some(x) => x, None => return Err(RhiError::Unsupported) };
        type PFN_vkCreateInstance = extern "system" fn(*const vkffi::VkInstanceCreateInfo, *const c_void, *mut vkffi::VkInstance) -> i32;
        let name_create: [i8; 16] = [118,107,67,114,101,97,116,101,73,110,115,116,97,110,99,101];
        let p = vkffi::load_global_ptr(&ldr, &name_create);
        if p.is_null() { return Err(RhiError::Unsupported) }
        let vkCreateInstance: PFN_vkCreateInstance = std::mem::transmute(p);
        let app = vkffi::VkApplicationInfo { sType: vkffi::VK_STRUCTURE_TYPE_APPLICATION_INFO, pNext: std::ptr::null(), pApplicationName: APP_NAME.as_ptr(), applicationVersion: 1, pEngineName: ENGINE_NAME.as_ptr(), engineVersion: 1, apiVersion: vkffi::VK_API_VERSION_1_0 };
        let exts: [*const i8; 2] = [EXT_SURFACE.as_ptr(), EXT_WIN32_SURFACE.as_ptr()];
        let ci = vkffi::VkInstanceCreateInfo { sType: vkffi::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, pNext: std::ptr::null(), flags: 0, pApplicationInfo: &app as *const _, enabledLayerCount: 0, ppEnabledLayerNames: std::ptr::null(), enabledExtensionCount: 2, ppEnabledExtensionNames: exts.as_ptr() };
        let mut instance: vkffi::VkInstance = std::ptr::null_mut();
        let r = vkCreateInstance(&ci as *const _, std::ptr::null(), &mut instance as *mut _);
        if r != 0 || instance.is_null() { Err(RhiError::Failed) } else { Ok(VkInstanceHandle(instance as *mut c_void)) }
    }
    #[cfg(not(target_os = "windows"))]
    { Err(RhiError::Unsupported) }
}

pub fn vk_destroy_instance(inst: VkInstanceHandle) -> Result<(), RhiError> {
    #[cfg(target_os = "windows")]
    unsafe {
        let ldr = match vkffi::load() { Some(x) => x, None => return Err(RhiError::Unsupported) };
        type PFN_vkDestroyInstance = extern "system" fn(vkffi::VkInstance, *const c_void);
        let name_destroy: [i8; 16] = [118,107,68,101,115,116,114,111,121,73,110,115,116,97,110,99];
        let p = vkffi::load_global_ptr(&ldr, &name_destroy);
        if p.is_null() { return Err(RhiError::Unsupported) }
        let vkDestroyInstance: PFN_vkDestroyInstance = std::mem::transmute(p);
        vkDestroyInstance(inst.0 as vkffi::VkInstance, std::ptr::null());
        Ok(())
    }
    #[cfg(not(target_os = "windows"))]
    { Err(RhiError::Unsupported) }
}

pub fn vk_create_surface(inst: VkInstanceHandle, hwnd: WindowHandle) -> Result<VulkanSurface, RhiError> {
    match create_vulkan_surface(inst.0, hwnd) { Ok(s) => Ok(s), Err(_) => Err(RhiError::Failed) }
}
