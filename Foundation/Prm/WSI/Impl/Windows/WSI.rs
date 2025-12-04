#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;
use crate::*;
use prm_window::WindowHandle;

#[repr(C)]
struct GUID { Data1: u32, Data2: u16, Data3: u16, Data4: [u8; 8] }

 

#[repr(C)]
struct IDXGIFactory1Vtbl {
    QueryInterface: extern "system" fn(*mut c_void, *const GUID, *mut *mut c_void) -> i32,
    AddRef: extern "system" fn(*mut c_void) -> u32,
    Release: extern "system" fn(*mut c_void) -> u32,
    _SetPrivateData: extern "system" fn(*mut c_void, *const GUID, u32, *const c_void) -> i32,
    _SetPrivateDataInterface: extern "system" fn(*mut c_void, *const GUID, *mut c_void) -> i32,
    _GetPrivateData: extern "system" fn(*mut c_void, *const GUID, *mut u32, *mut c_void) -> i32,
    _GetParent: extern "system" fn(*mut c_void, *const GUID, *mut *mut c_void) -> i32,
    EnumAdapters: extern "system" fn(*mut c_void, u32, *mut *mut c_void) -> i32,
    MakeWindowAssociation: extern "system" fn(*mut c_void, *mut c_void, u32) -> i32,
    GetWindowAssociation: extern "system" fn(*mut c_void, *mut *mut c_void) -> i32,
    CreateSwapChain: extern "system" fn(*mut c_void, *mut c_void, *mut c_void, *mut *mut c_void) -> i32,
    CreateSoftwareAdapter: extern "system" fn(*mut c_void, *mut c_void, *mut *mut c_void) -> i32,
    EnumAdapters1: extern "system" fn(*mut c_void, u32, *mut *mut c_void) -> i32,
    IsCurrent: extern "system" fn(*mut c_void) -> i32,
}

#[repr(C)]
struct IDXGIFactory1 { lpVtbl: *const IDXGIFactory1Vtbl }

#[repr(C)]
struct LUID { LowPart: u32, HighPart: i32 }

#[repr(C)]
struct DXGI_ADAPTER_DESC1 {
    Description: [u16; 128],
    VendorId: u32,
    DeviceId: u32,
    SubSysId: u32,
    Revision: u32,
    DedicatedVideoMemory: usize,
    DedicatedSystemMemory: usize,
    SharedSystemMemory: usize,
    AdapterLuid: LUID,
    Flags: u32,
}

#[repr(C)]
struct IDXGIAdapter1Vtbl {
    QueryInterface: extern "system" fn(*mut c_void, *const GUID, *mut *mut c_void) -> i32,
    AddRef: extern "system" fn(*mut c_void) -> u32,
    Release: extern "system" fn(*mut c_void) -> u32,
    _SetPrivateData: extern "system" fn(*mut c_void, *const GUID, u32, *const c_void) -> i32,
    _SetPrivateDataInterface: extern "system" fn(*mut c_void, *const GUID, *mut c_void) -> i32,
    _GetPrivateData: extern "system" fn(*mut c_void, *const GUID, *mut u32, *mut c_void) -> i32,
    _GetParent: extern "system" fn(*mut c_void, *const GUID, *mut *mut c_void) -> i32,
    EnumOutputs: extern "system" fn(*mut c_void, u32, *mut *mut c_void) -> i32,
    GetDesc: extern "system" fn(*mut c_void, *mut c_void) -> i32,
    CheckInterfaceSupport: extern "system" fn(*mut c_void, *const GUID, *mut i64) -> i32,
    GetDesc1: extern "system" fn(*mut c_void, *mut DXGI_ADAPTER_DESC1) -> i32,
}

#[repr(C)]
struct IDXGIAdapter1 { lpVtbl: *const IDXGIAdapter1Vtbl }

#[link(name = "dxgi")]
extern "system" { fn CreateDXGIFactory1(riid: *const GUID, ppFactory: *mut *mut c_void) -> i32; }

const IID_IDXGIFactory1: GUID = GUID { Data1: 0x770aae78, Data2: 0xf26f, Data3: 0x4dba, Data4: [0xa8,0x29,0x25,0x3c,0x83,0xd1,0xb3,0x87] };

fn wide_to_string(w: &[u16]) -> String {
    let mut len = 0usize;
    while len < w.len() { if w[len] == 0 { break; } len += 1; }
    String::from_utf16_lossy(&w[..len])
}

fn to_wide(s: &str) -> Vec<u16> {
    let mut v: Vec<u16> = s.encode_utf16().collect();
    v.push(0);
    v
}

pub fn impl_enumerate_adapters() -> Result<Vec<AdapterInfo>, WSIError> {
    unsafe {
        let mut p_factory: *mut c_void = std::ptr::null_mut();
        let hr = CreateDXGIFactory1(&IID_IDXGIFactory1 as *const GUID, &mut p_factory as *mut *mut c_void);
        if hr < 0 || p_factory.is_null() { return Err(WSIError::Failed); }
        let factory = p_factory as *mut IDXGIFactory1;

        let mut list: Vec<AdapterInfo> = Vec::new();
        let mut index: u32 = 0;
        loop {
            let mut p_adapter: *mut c_void = std::ptr::null_mut();
            let hr = ((*(*factory).lpVtbl).EnumAdapters1)(factory as *mut c_void, index, &mut p_adapter as *mut *mut c_void);
            if hr < 0 { break; }
            if p_adapter.is_null() { break; }
            let adapter = p_adapter as *mut IDXGIAdapter1;

            let mut desc: DXGI_ADAPTER_DESC1 = std::mem::zeroed();
            let hr = ((*(*adapter).lpVtbl).GetDesc1)(adapter as *mut c_void, &mut desc as *mut DXGI_ADAPTER_DESC1);
            if hr >= 0 {
                let name = wide_to_string(&desc.Description);
                let info = AdapterInfo { name, vendor_id: desc.VendorId, device_id: desc.DeviceId, dedicated_video_memory: desc.DedicatedVideoMemory as u64, flags: desc.Flags };
                list.push(info);
            }

            let _ = ((*(*adapter).lpVtbl).Release)(adapter as *mut c_void);
            index += 1;
        }

        let _ = ((*(*factory).lpVtbl).Release)(factory as *mut c_void);
        if list.is_empty() { Err(WSIError::Failed) } else { Ok(list) }
    }
}

type HWND = *mut c_void;
type HDC = *mut c_void;
type HBITMAP = *mut c_void;

#[repr(C)]
struct BITMAPINFOHEADER {
    biSize: u32,
    biWidth: i32,
    biHeight: i32,
    biPlanes: u16,
    biBitCount: u16,
    biCompression: u32,
    biSizeImage: u32,
    biXPelsPerMeter: i32,
    biYPelsPerMeter: i32,
    biClrUsed: u32,
    biClrImportant: u32,
}

#[repr(C)]
struct BITMAPINFO {
    bmiHeader: BITMAPINFOHEADER,
}

#[link(name = "user32")]
extern "system" {
    fn GetDC(hWnd: HWND) -> HDC;
    fn ReleaseDC(hWnd: HWND, hDC: HDC) -> i32;
}

#[link(name = "gdi32")]
extern "system" {
    fn CreateCompatibleDC(hdc: HDC) -> HDC;
    fn DeleteDC(hdc: HDC) -> i32;
    fn CreateDIBSection(hdc: HDC, pbmi: *const BITMAPINFO, iUsage: u32, ppvBits: *mut *mut c_void, hSection: *mut c_void, dwOffset: u32) -> HBITMAP;
    fn DeleteObject(hObject: HBITMAP) -> i32;
    fn SelectObject(hdc: HDC, h: HBITMAP) -> HBITMAP;
    fn BitBlt(hdc: HDC, x: i32, y: i32, cx: i32, cy: i32, hdcSrc: HDC, x1: i32, y1: i32, rop: u32) -> i32;
}

const BI_RGB: u32 = 0;
const DIB_RGB_COLORS: u32 = 0;
const SRCCOPY: u32 = 0x00CC0020;

struct CpuPresentState { hwnd: HWND, hdc: HDC, memdc: HDC, bmp: HBITMAP, pixels: *mut c_void, pitch: u32, w: u32, h: u32 }

pub fn impl_create_cpu_present(hwnd: WindowHandle, width: u32, height: u32) -> Result<CpuPresentHandle, WSIError> {
    unsafe {
        let mut s = Box::new(CpuPresentState { hwnd: hwnd.0, hdc: std::ptr::null_mut(), memdc: std::ptr::null_mut(), bmp: std::ptr::null_mut(), pixels: std::ptr::null_mut(), pitch: 0, w: width, h: height });
        s.hdc = if s.hwnd.is_null() { std::ptr::null_mut() } else { GetDC(s.hwnd) };
        if s.hdc.is_null() { return Err(WSIError::Failed); }
        s.memdc = CreateCompatibleDC(s.hdc);
        if s.memdc.is_null() { ReleaseDC(s.hwnd, s.hdc); return Err(WSIError::Failed); }
        s.pitch = s.w * 4;
        let bmi = BITMAPINFO { bmiHeader: BITMAPINFOHEADER { biSize: std::mem::size_of::<BITMAPINFOHEADER>() as u32, biWidth: s.w as i32, biHeight: -(s.h as i32), biPlanes: 1, biBitCount: 32, biCompression: BI_RGB, biSizeImage: 0, biXPelsPerMeter: 0, biYPelsPerMeter: 0, biClrUsed: 0, biClrImportant: 0 } };
        let mut pixels: *mut c_void = std::ptr::null_mut();
        s.bmp = CreateDIBSection(s.memdc, &bmi as *const BITMAPINFO, DIB_RGB_COLORS, &mut pixels as *mut *mut c_void, std::ptr::null_mut(), 0);
        if s.bmp.is_null() || pixels.is_null() {
            if !s.bmp.is_null() { let _ = DeleteObject(s.bmp); }
            if !s.memdc.is_null() { let _ = DeleteDC(s.memdc); }
            if !s.hdc.is_null() && !s.hwnd.is_null() { let _ = ReleaseDC(s.hwnd, s.hdc); }
            return Err(WSIError::Failed);
        }
        s.pixels = pixels;
        let _ = SelectObject(s.memdc, s.bmp);
        Ok(CpuPresentHandle(Box::into_raw(s) as *mut c_void))
    }
}

pub fn impl_destroy_cpu_present(state: CpuPresentHandle) -> Result<(), WSIError> {
    unsafe {
        if state.0.is_null() { return Err(WSIError::Failed); }
        let s = Box::from_raw(state.0 as *mut CpuPresentState);
        if !s.bmp.is_null() { let _ = DeleteObject(s.bmp); }
        if !s.memdc.is_null() { let _ = DeleteDC(s.memdc); }
        if !s.hdc.is_null() && !s.hwnd.is_null() { let _ = ReleaseDC(s.hwnd, s.hdc); }
        Ok(())
    }
}

pub fn impl_cpu_get_buffer(state: CpuPresentHandle) -> Result<*mut u8, WSIError> {
    unsafe {
        if state.0.is_null() { return Err(WSIError::Failed); }
        let s = &*(state.0 as *mut CpuPresentState);
        if s.pixels.is_null() { return Err(WSIError::Failed); }
        Ok(s.pixels as *mut u8)
    }
}

pub fn impl_cpu_get_pitch(state: CpuPresentHandle) -> Result<u32, WSIError> {
    unsafe {
        if state.0.is_null() { return Err(WSIError::Failed); }
        let s = &*(state.0 as *mut CpuPresentState);
        Ok(s.pitch)
    }
}

pub fn impl_cpu_present(state: CpuPresentHandle) -> Result<(), WSIError> {
    unsafe {
        if state.0.is_null() { return Err(WSIError::Failed); }
        let s = &*(state.0 as *mut CpuPresentState);
        if !s.hdc.is_null() && !s.memdc.is_null() { let _ = BitBlt(s.hdc, 0, 0, s.w as i32, s.h as i32, s.memdc, 0, 0, SRCCOPY); }
        Ok(())
    }
}

#[link(name = "kernel32")]
extern "system" {
    fn LoadLibraryW(lpLibFileName: *const u16) -> *mut c_void;
    fn GetProcAddress(hModule: *mut c_void, lpProcName: *const i8) -> *mut c_void;
}

#[link(name = "user32")]
extern "system" { fn GetModuleHandleW(lpModuleName: *const u16) -> *mut c_void; }

type VkInstance = *mut c_void;
type VkSurfaceKHR = *mut c_void;

#[repr(C)]
struct VkWin32SurfaceCreateInfoKHR { sType: u32, pNext: *const c_void, flags: u32, hinstance: *mut c_void, hwnd: HWND }

const VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR: u32 = 1000009000;

pub fn impl_create_vulkan_surface(instance: *mut c_void, hwnd: WindowHandle) -> Result<VulkanSurface, WSIError> {
    unsafe {
        let mut hlib = LoadLibraryW(to_wide("vulkan-1.dll").as_ptr());
        if hlib.is_null() { hlib = LoadLibraryW(to_wide("D\\Code\\Vulkan\\Bin\\vulkan-1.dll").as_ptr()); }
        if hlib.is_null() { hlib = LoadLibraryW(to_wide("D\\Code\\Vulkan\\Bin\\Win64\\vulkan-1.dll").as_ptr()); }
        if hlib.is_null() { return Err(WSIError::Unsupported); }
        let name_gipa: [i8; 23] = [118,107,71,101,116,73,110,115,116,97,110,99,101,80,114,111,99,65,100,100,114,0,0];
        let gipa = GetProcAddress(hlib, name_gipa.as_ptr());
        if gipa.is_null() { return Err(WSIError::Unsupported); }
        let vkGetInstanceProcAddr: extern "system" fn(VkInstance, *const i8) -> *mut c_void = std::mem::transmute(gipa);
        let name_create: [i8; 24] = [118,107,67,114,101,97,116,101,87,105,110,51,50,83,117,114,102,97,99,101,75,72,82,0];
        let fptr = vkGetInstanceProcAddr(instance as VkInstance, name_create.as_ptr());
        if fptr.is_null() { return Err(WSIError::Unsupported); }
        let vkCreateWin32SurfaceKHR: extern "system" fn(VkInstance, *const VkWin32SurfaceCreateInfoKHR, *const c_void, *mut VkSurfaceKHR) -> i32 = std::mem::transmute(fptr);
        let hinst = GetModuleHandleW(std::ptr::null());
        let sci = VkWin32SurfaceCreateInfoKHR { sType: VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, pNext: std::ptr::null(), flags: 0, hinstance: hinst, hwnd: hwnd.0 };
        let mut surface: VkSurfaceKHR = std::ptr::null_mut();
        let r = vkCreateWin32SurfaceKHR(instance as VkInstance, &sci as *const VkWin32SurfaceCreateInfoKHR, std::ptr::null(), &mut surface as *mut VkSurfaceKHR);
        if r != 0 || surface.is_null() { return Err(WSIError::Failed); }
        Ok(VulkanSurface(surface))
    }
}

pub fn impl_destroy_vulkan_surface(instance: *mut c_void, surface: VulkanSurface) -> Result<(), WSIError> {
    unsafe {
        let mut hlib = LoadLibraryW(to_wide("vulkan-1.dll").as_ptr());
        if hlib.is_null() { hlib = LoadLibraryW(to_wide("D\\Code\\Vulkan\\Bin\\Win64\\vulkan-1.dll").as_ptr()); }
        if hlib.is_null() { return Err(WSIError::Unsupported); }
        let name_gipa: [i8; 23] = [118,107,71,101,116,73,110,115,116,97,110,99,101,80,114,111,99,65,100,100,114,0,0];
        let gipa = GetProcAddress(hlib, name_gipa.as_ptr());
        if gipa.is_null() { return Err(WSIError::Unsupported); }
        let vkGetInstanceProcAddr: extern "system" fn(VkInstance, *const i8) -> *mut c_void = std::mem::transmute(gipa);
        let name_destroy: [i8; 21] = [118,107,68,101,115,116,114,111,121,83,117,114,102,97,99,101,75,72,82,0,0];
        let fptr = vkGetInstanceProcAddr(instance as VkInstance, name_destroy.as_ptr());
        if fptr.is_null() { return Err(WSIError::Unsupported); }
        let vkDestroySurfaceKHR: extern "system" fn(VkInstance, VkSurfaceKHR, *const c_void) -> i32 = std::mem::transmute(fptr);
        let r = vkDestroySurfaceKHR(instance as VkInstance, surface.0 as VkSurfaceKHR, std::ptr::null());
        if r != 0 { return Err(WSIError::Failed); }
        Ok(())
    }
}
