use std::ffi::c_void;

#[link(name = "kernel32")]
extern "system" {
    fn LoadLibraryW(lpLibFileName: *const u16) -> *mut c_void;
    fn GetProcAddress(hModule: *mut c_void, lpProcName: *const i8) -> *mut c_void;
}

pub type VkInstance = *mut c_void;

pub struct VulkanLoader { pub hlib: *mut c_void, pub gpa: extern "system" fn(VkInstance, *const i8) -> *mut c_void }

pub fn load() -> Option<VulkanLoader> {
    let name: [u16; 11] = ['v' as u16,'u' as u16,'l' as u16,'k' as u16,'a' as u16,'n' as u16,'-' as u16,'1' as u16,'.' as u16,'d' as u16,'l' as u16];
    unsafe {
        let h = LoadLibraryW(name.as_ptr());
        if h.is_null() { return None; }
        let gipa: [i8; 23] = [118,107,71,101,116,73,110,115,116,97,110,99,101,80,114,111,99,65,100,100,114,0,0];
        let p = GetProcAddress(h, gipa.as_ptr());
        if p.is_null() { return None; }
        let f: extern "system" fn(VkInstance, *const i8) -> *mut c_void = std::mem::transmute(p);
        Some(VulkanLoader { hlib: h, gpa: f })
    }
}

pub unsafe fn load_global_ptr(ldr: &VulkanLoader, name: &[i8]) -> *mut c_void {
    (ldr.gpa)(std::ptr::null_mut(), name.as_ptr())
}

#[repr(C)]
pub struct VkApplicationInfo { pub sType: u32, pub pNext: *const c_void, pub pApplicationName: *const i8, pub applicationVersion: u32, pub pEngineName: *const i8, pub engineVersion: u32, pub apiVersion: u32 }

#[repr(C)]
pub struct VkInstanceCreateInfo { pub sType: u32, pub pNext: *const c_void, pub flags: u32, pub pApplicationInfo: *const VkApplicationInfo, pub enabledLayerCount: u32, pub ppEnabledLayerNames: *const *const i8, pub enabledExtensionCount: u32, pub ppEnabledExtensionNames: *const *const i8 }

pub const VK_STRUCTURE_TYPE_APPLICATION_INFO: u32 = 0;
pub const VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO: u32 = 1;
pub const VK_API_VERSION_1_0: u32 = (1 << 22) | (0 << 12) | 0;
