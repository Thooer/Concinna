#![allow(non_snake_case, non_camel_case_types)]
use std::ffi::c_void;
use std::ptr;
use crate::*;
use std::sync::OnceLock;

type HWND = *mut c_void;
type HINSTANCE = *mut c_void;
type HICON = *mut c_void;
type HCURSOR = *mut c_void;
type HBRUSH = *mut c_void;

#[repr(C)]
struct WNDCLASSEXW {
    cbSize: u32,
    style: u32,
    lpfnWndProc: extern "system" fn(HWND, u32, usize, isize) -> isize,
    cbClsExtra: i32,
    cbWndExtra: i32,
    hInstance: HINSTANCE,
    hIcon: HICON,
    hCursor: HCURSOR,
    hbrBackground: HBRUSH,
    lpszMenuName: *const u16,
    lpszClassName: *const u16,
    hIconSm: HICON,
}

#[repr(C)]
struct MSG {
    hwnd: HWND,
    message: u32,
    wParam: usize,
    lParam: isize,
    time: u32,
    pt_x: i32,
    pt_y: i32,
}

const CS_HREDRAW: u32 = 0x0002;
const CS_VREDRAW: u32 = 0x0001;
const WS_OVERLAPPEDWINDOW: u32 = 0x00CF0000;
const WS_VISIBLE: u32 = 0x10000000;
const SW_SHOW: i32 = 5;
const PM_REMOVE: u32 = 0x0001;

#[link(name = "user32")]
extern "system" {
    fn RegisterClassExW(lpwcx: *const WNDCLASSEXW) -> u16;
    fn CreateWindowExW(dwExStyle: u32, lpClassName: *const u16, lpWindowName: *const u16, dwStyle: u32, x: i32, y: i32, nWidth: i32, nHeight: i32, hWndParent: HWND, hMenu: *mut c_void, hInstance: HINSTANCE, lpParam: *mut c_void) -> HWND;
    fn DefWindowProcW(hWnd: HWND, Msg: u32, wParam: usize, lParam: isize) -> isize;
    fn ShowWindow(hWnd: HWND, nCmdShow: i32) -> i32;
    fn UpdateWindow(hWnd: HWND) -> i32;
    fn DestroyWindow(hWnd: HWND) -> i32;
    fn SetWindowTextW(hWnd: HWND, lpString: *const u16) -> i32;
    fn PeekMessageW(lpMsg: *mut MSG, hWnd: HWND, wMsgFilterMin: u32, wMsgFilterMax: u32, wRemoveMsg: u32) -> i32;
    fn TranslateMessage(lpMsg: *const MSG) -> i32;
    fn DispatchMessageW(lpMsg: *const MSG) -> isize;
    fn LoadCursorW(hInstance: HINSTANCE, lpCursorName: *const u16) -> HCURSOR;
    fn ShowCursor(bShow: i32) -> i32;
    fn ClipCursor(lprc: *const RECT) -> i32;
    fn GetClientRect(hWnd: HWND, lpRect: *mut RECT) -> i32;
    fn ClientToScreen(hWnd: HWND, lpPoint: *mut POINT) -> i32;
    fn SetCursorPos(X: i32, Y: i32) -> i32;
    fn SetWindowPos(hWnd: HWND, hWndInsertAfter: HWND, X: i32, Y: i32, cx: i32, cy: i32, uFlags: u32) -> i32;
}

#[link(name = "kernel32")]
extern "system" {
    fn MultiByteToWideChar(CodePage: u32, dwFlags: u32, lpMultiByteStr: *const i8, cbMultiByte: i32, lpWideCharStr: *mut u16, cchWideChar: i32) -> i32;
}

static RAW_CB: OnceLock<Option<RawEventCallback>> = OnceLock::new();
static USER_WNDPROC: OnceLock<Option<WndProcCallback>> = OnceLock::new();
static CLASS_NAME_W: [u16; 10] = [
    'P' as u16,
    'r' as u16,
    'm' as u16,
    'W' as u16,
    'i' as u16,
    'n' as u16,
    'd' as u16,
    'o' as u16,
    'w' as u16,
    0u16,
];

extern "system" fn wndproc(hwnd: HWND, msg: u32, wparam: usize, lparam: isize) -> isize {
    if let Some(Some(cb)) = USER_WNDPROC.get() { return cb(hwnd, msg, wparam, lparam) as isize; }
    unsafe { DefWindowProcW(hwnd, msg, wparam, lparam) }
}

fn class_name() -> *const u16 { CLASS_NAME_W.as_ptr() }

pub fn impl_set_raw_event_callback(cb: Option<RawEventCallback>) {
    let _ = RAW_CB.set(cb);
}

pub fn impl_create(desc: &WindowDesc, cb: Option<WndProcCallback>) -> Result<WindowHandle, WindowError> {
    let _ = USER_WNDPROC.set(cb);
    unsafe {
        let wc = WNDCLASSEXW { cbSize: std::mem::size_of::<WNDCLASSEXW>() as u32, style: CS_HREDRAW | CS_VREDRAW, lpfnWndProc: wndproc, cbClsExtra: 0, cbWndExtra: 0, hInstance: ptr::null_mut(), hIcon: ptr::null_mut(), hCursor: LoadCursorW(ptr::null_mut(), 32512u16 as *const u16), hbrBackground: ptr::null_mut(), lpszMenuName: ptr::null(), lpszClassName: class_name(), hIconSm: ptr::null_mut() };
        if RegisterClassExW(&wc as *const WNDCLASSEXW) == 0 { return Err(WindowError::Failed); }
        let title = [
            'C' as u16,'o' as u16,'n' as u16,'c' as u16,'i' as u16,'n' as u16,'n' as u16,'a' as u16,0u16
        ];
        let style = if desc.visible { WS_OVERLAPPEDWINDOW | WS_VISIBLE } else { WS_OVERLAPPEDWINDOW };
        let hwnd = CreateWindowExW(0, class_name(), title.as_ptr(), style, 100, 100, desc.width as i32, desc.height as i32, ptr::null_mut(), ptr::null_mut(), ptr::null_mut(), ptr::null_mut());
        if hwnd.is_null() { return Err(WindowError::Failed); }
        UpdateWindow(hwnd);
        Ok(WindowHandle(hwnd))
    }
}

pub fn impl_destroy(h: WindowHandle) { unsafe { DestroyWindow(h.0); } }

pub fn impl_show(h: WindowHandle) -> Result<(), WindowError> { unsafe { ShowWindow(h.0, SW_SHOW); UpdateWindow(h.0); } Ok(()) }

pub fn impl_hide(_h: WindowHandle) -> Result<(), WindowError> { Ok(()) }

pub fn impl_set_title(h: WindowHandle, title: &str) -> Result<(), WindowError> {
    const CP_UTF8: u32 = 65001;
    let src = title.as_bytes();
    let mut len = unsafe { MultiByteToWideChar(CP_UTF8, 0, src.as_ptr() as *const i8, src.len() as i32, std::ptr::null_mut(), 0) };
    if len <= 0 { return Err(WindowError::Failed); }
    len += 1;
    let mut w = vec![0u16; len as usize];
    let wrote = unsafe { MultiByteToWideChar(CP_UTF8, 0, src.as_ptr() as *const i8, src.len() as i32, w.as_mut_ptr(), len - 1) };
    if wrote <= 0 { return Err(WindowError::Failed); }
    w[wrote as usize] = 0;
    unsafe { if SetWindowTextW(h.0, w.as_ptr()) == 0 { return Err(WindowError::Failed); } }
    Ok(())
}

const SWP_NOMOVE: u32 = 0x0002;
const SWP_NOZORDER: u32 = 0x0004;
pub fn impl_resize(h: WindowHandle, w: u32, hgt: u32) -> Result<(), WindowError> {
    unsafe {
        if SetWindowPos(h.0, ptr::null_mut(), 0, 0, w as i32, hgt as i32, SWP_NOMOVE | SWP_NOZORDER) == 0 {
            return Err(WindowError::Failed);
        }
    }
    Ok(())
}

#[repr(C)]
struct RECT { left: i32, top: i32, right: i32, bottom: i32 }
#[repr(C)]
struct POINT { x: i32, y: i32 }

pub fn impl_set_cursor_mode(h: WindowHandle, m: CursorMode) -> Result<(), WindowError> {
    unsafe {
        match m {
            CursorMode::Normal => {
                ShowCursor(1);
                ClipCursor(ptr::null());
            }
            CursorMode::Hidden => {
                ShowCursor(0);
                ClipCursor(ptr::null());
            }
            CursorMode::Locked => {
                ShowCursor(0);
                let mut rc = RECT { left: 0, top: 0, right: 0, bottom: 0 };
                if GetClientRect(h.0, &mut rc as *mut RECT) == 0 { return Err(WindowError::Failed); }
                let mut tl = POINT { x: rc.left, y: rc.top };
                let mut br = POINT { x: rc.right, y: rc.bottom };
                if ClientToScreen(h.0, &mut tl as *mut POINT) == 0 { return Err(WindowError::Failed); }
                if ClientToScreen(h.0, &mut br as *mut POINT) == 0 { return Err(WindowError::Failed); }
                let clip = RECT { left: tl.x, top: tl.y, right: br.x, bottom: br.y };
                if ClipCursor(&clip as *const RECT) == 0 { return Err(WindowError::Failed); }
                let cx = (clip.left + clip.right) / 2;
                let cy = (clip.top + clip.bottom) / 2;
                let _ = SetCursorPos(cx, cy);
            }
        }
    }
    Ok(())
}

pub fn impl_process_one_message(_h: Option<WindowHandle>) -> bool {
    unsafe {
        let mut msg = MSG { hwnd: ptr::null_mut(), message: 0, wParam: 0, lParam: 0, time: 0, pt_x: 0, pt_y: 0 };
        let has = PeekMessageW(&mut msg as *mut MSG, ptr::null_mut(), 0, 0, PM_REMOVE);
        if has != 0 {
            if let Some(Some(cb)) = RAW_CB.get() { cb(msg.hwnd, msg.message, msg.wParam, msg.lParam); }
            TranslateMessage(&msg as *const MSG);
            DispatchMessageW(&msg as *const MSG);
            return true;
        }
        false
    }
}

pub fn impl_native(h: WindowHandle) -> Result<*mut c_void, WindowError> { Ok(h.0) }
