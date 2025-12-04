#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;
use std::sync::{Mutex, OnceLock};
use crate::*;

type HWND = *mut c_void;
type HRAWINPUT = *mut c_void;

#[repr(C)]
struct RAWINPUTHEADER { dwType: u32, dwSize: u32, hDevice: *mut c_void, wParam: usize }

#[repr(C)]
#[derive(Clone, Copy)]
struct RAWMOUSE { usFlags: u16, ulButtons: u32, usButtonFlags: u16, usButtonData: u16, ulRawButtons: u32, lLastX: i32, lLastY: i32, ulExtraInformation: u32 }

#[repr(C)]
#[derive(Clone, Copy)]
struct RAWKEYBOARD { MakeCode: u16, Flags: u16, Reserved: u16, VKey: u16, Message: u32, ExtraInformation: u32 }

#[repr(C)]
struct RAWINPUT { header: RAWINPUTHEADER, data: RAWINPUTUNION }

#[repr(C)]
union RAWINPUTUNION { mouse: RAWMOUSE, keyboard: RAWKEYBOARD }

const RIDEV_INPUTSINK: u32 = 0x00000100;
const RIM_TYPEMOUSE: u32 = 0;
const RIM_TYPEKEYBOARD: u32 = 1;
const WM_INPUT: u32 = 0x00FF;

const RI_KEY_MAKE: u16 = 0;
const RI_KEY_BREAK: u16 = 1;

const MOUSE_MOVE_RELATIVE: u16 = 0;
const RI_MOUSE_LEFT_BUTTON_DOWN: u16 = 0x0001;
const RI_MOUSE_LEFT_BUTTON_UP: u16 = 0x0002;
const RI_MOUSE_RIGHT_BUTTON_DOWN: u16 = 0x0004;
const RI_MOUSE_RIGHT_BUTTON_UP: u16 = 0x0008;
const RI_MOUSE_MIDDLE_BUTTON_DOWN: u16 = 0x0010;
const RI_MOUSE_MIDDLE_BUTTON_UP: u16 = 0x0020;
const RI_MOUSE_WHEEL: u16 = 0x0400;

#[repr(C)]
struct RAWINPUTDEVICE { usUsagePage: u16, usUsage: u16, dwFlags: u32, hwndTarget: HWND }

#[link(name = "user32")]
extern "system" {
    fn RegisterRawInputDevices(pRawInputDevices: *const RAWINPUTDEVICE, cDevices: u32, cbSize: u32) -> i32;
    fn GetRawInputData(hRawInput: HRAWINPUT, uiCommand: u32, pData: *mut c_void, pcbSize: *mut u32, cbSizeHeader: u32) -> u32;
}

static QUEUE: OnceLock<Mutex<Vec<HidEvent>>> = OnceLock::new();

pub fn impl_register_devices(hwnd: RawWindowHandle) -> Result<(), HidError> {
    let d = [
        RAWINPUTDEVICE { usUsagePage: 0x01, usUsage: 0x02, dwFlags: RIDEV_INPUTSINK, hwndTarget: hwnd as HWND },
        RAWINPUTDEVICE { usUsagePage: 0x01, usUsage: 0x06, dwFlags: RIDEV_INPUTSINK, hwndTarget: hwnd as HWND },
    ];
    let ok = unsafe { RegisterRawInputDevices(d.as_ptr(), d.len() as u32, std::mem::size_of::<RAWINPUTDEVICE>() as u32) };
    if ok == 0 { return Err(HidError::Failed); }
    let _ = QUEUE.set(Mutex::new(Vec::new()));
    Ok(())
}

pub fn impl_on_raw_event(_hwnd: RawWindowHandle, msg: u32, _wparam: usize, lparam: isize) {
    if msg != WM_INPUT { return; }
    let mut sz: u32 = 0;
    let got = unsafe { GetRawInputData(lparam as HRAWINPUT, 0x10000003u32, std::ptr::null_mut(), &mut sz as *mut u32, std::mem::size_of::<RAWINPUTHEADER>() as u32) };
    if got == 0 && sz > 0 {
        let mut buf = vec![0u8; sz as usize];
        let r = unsafe { GetRawInputData(lparam as HRAWINPUT, 0x10000003u32, buf.as_mut_ptr() as *mut c_void, &mut sz as *mut u32, std::mem::size_of::<RAWINPUTHEADER>() as u32) };
        if r == sz {
            let ri = unsafe { &*(buf.as_ptr() as *const RAWINPUT) };
            let mut q = QUEUE.get().unwrap().lock().unwrap();
            match ri.header.dwType {
                RIM_TYPEKEYBOARD => {
                    let kb = unsafe { &ri.data.keyboard };
                    let v = kb.VKey;
                    let f = kb.Flags;
                    if f == RI_KEY_MAKE { q.push(HidEvent::KeyDown(v)); }
                    else if f == RI_KEY_BREAK { q.push(HidEvent::KeyUp(v)); }
                }
                RIM_TYPEMOUSE => {
                    let m = unsafe { &ri.data.mouse };
                    if m.usFlags == MOUSE_MOVE_RELATIVE { if m.lLastX != 0 || m.lLastY != 0 { q.push(HidEvent::MouseMove(m.lLastX, m.lLastY)); } }
                    let b = m.usButtonFlags;
                    if (b & RI_MOUSE_LEFT_BUTTON_DOWN) != 0 { q.push(HidEvent::MouseLeftDown); }
                    if (b & RI_MOUSE_LEFT_BUTTON_UP) != 0 { q.push(HidEvent::MouseLeftUp); }
                    if (b & RI_MOUSE_RIGHT_BUTTON_DOWN) != 0 { q.push(HidEvent::MouseRightDown); }
                    if (b & RI_MOUSE_RIGHT_BUTTON_UP) != 0 { q.push(HidEvent::MouseRightUp); }
                    if (b & RI_MOUSE_MIDDLE_BUTTON_DOWN) != 0 { q.push(HidEvent::MouseMiddleDown); }
                    if (b & RI_MOUSE_MIDDLE_BUTTON_UP) != 0 { q.push(HidEvent::MouseMiddleUp); }
                    if (b & RI_MOUSE_WHEEL) != 0 { q.push(HidEvent::MouseWheel(m.usButtonData as i16)); }
                }
                _ => {}
            }
            drop(q);
        }
    }
}

pub fn impl_take_events(out: &mut [HidEvent]) -> usize {
    if let Some(m) = QUEUE.get() {
        let mut q = m.lock().unwrap();
        let n = out.len().min(q.len());
        for i in 0..n { out[i] = q[i]; }
        q.drain(0..n);
        n
    } else { 0 }
}
