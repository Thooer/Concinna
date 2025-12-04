#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;

#[repr(C)]
struct GUID { Data1: u32, Data2: u16, Data3: u16, Data4: [u8; 8] }

#[repr(C)]
struct WAVEFORMATEX {
    wFormatTag: u16,
    nChannels: u16,
    nSamplesPerSec: u32,
    nAvgBytesPerSec: u32,
    nBlockAlign: u16,
    wBitsPerSample: u16,
    cbSize: u16,
}

const WAVE_FORMAT_PCM: u16 = 0x0001;
const WAVE_FORMAT_IEEE_FLOAT: u16 = 0x0003;

 

#[repr(C)]
struct IMMDeviceEnumeratorVtbl {
    // IUnknown
    QueryInterface: extern "system" fn(*mut c_void, *const GUID, *mut *mut c_void) -> i32,
    AddRef: extern "system" fn(*mut c_void) -> u32,
    Release: extern "system" fn(*mut c_void) -> u32,
    // IMMDeviceEnumerator
    EnumAudioEndpoints: extern "system" fn(*mut c_void, u32, u32, *mut *mut c_void) -> i32,
    GetDefaultAudioEndpoint: extern "system" fn(*mut c_void, u32, u32, *mut *mut c_void) -> i32,
    GetDevice: extern "system" fn(*mut c_void, *const u16, *mut *mut c_void) -> i32,
    RegisterEndpointNotificationCallback: extern "system" fn(*mut c_void, *mut c_void) -> i32,
    UnregisterEndpointNotificationCallback: extern "system" fn(*mut c_void, *mut c_void) -> i32,
}

#[repr(C)]
struct IMMDeviceEnumerator { lpVtbl: *const IMMDeviceEnumeratorVtbl }

#[repr(C)]
struct IMMDeviceVtbl {
    // IUnknown
    QueryInterface: extern "system" fn(*mut c_void, *const GUID, *mut *mut c_void) -> i32,
    AddRef: extern "system" fn(*mut c_void) -> u32,
    Release: extern "system" fn(*mut c_void) -> u32,
    // IMMDevice
    Activate: extern "system" fn(*mut c_void, *const GUID, u32, *mut c_void, *mut *mut c_void) -> i32,
    OpenPropertyStore: extern "system" fn(*mut c_void, u32, *mut *mut c_void) -> i32,
    GetId: extern "system" fn(*mut c_void, *mut *mut u16) -> i32,
    GetState: extern "system" fn(*mut c_void, *mut u32) -> i32,
}

#[repr(C)]
struct IMMDevice { lpVtbl: *const IMMDeviceVtbl }

#[repr(C)]
struct IAudioClientVtbl {
    // IUnknown
    QueryInterface: extern "system" fn(*mut c_void, *const GUID, *mut *mut c_void) -> i32,
    AddRef: extern "system" fn(*mut c_void) -> u32,
    Release: extern "system" fn(*mut c_void) -> u32,
    // IAudioClient (method order per SDK)
    Initialize: extern "system" fn(*mut c_void, u32, u32, i64, i64, *const WAVEFORMATEX, *const GUID) -> i32,
    GetBufferSize: extern "system" fn(*mut c_void, *mut u32) -> i32,
    GetStreamLatency: extern "system" fn(*mut c_void, *mut i64) -> i32,
    GetCurrentPadding: extern "system" fn(*mut c_void, *mut u32) -> i32,
    IsFormatSupported: extern "system" fn(*mut c_void, u32, *const WAVEFORMATEX, *mut *mut WAVEFORMATEX) -> i32,
    GetMixFormat: extern "system" fn(*mut c_void, *mut *mut WAVEFORMATEX) -> i32,
    GetDevicePeriod: extern "system" fn(*mut c_void, *mut i64, *mut i64) -> i32,
    Start: extern "system" fn(*mut c_void) -> i32,
    Stop: extern "system" fn(*mut c_void) -> i32,
    Reset: extern "system" fn(*mut c_void) -> i32,
    SetEventHandle: extern "system" fn(*mut c_void, *mut c_void) -> i32,
    GetService: extern "system" fn(*mut c_void, *const GUID, *mut *mut c_void) -> i32,
}

#[repr(C)]
struct IAudioClient { lpVtbl: *const IAudioClientVtbl }

#[repr(C)]
struct IAudioRenderClientVtbl {
    // IUnknown
    QueryInterface: extern "system" fn(*mut c_void, *const GUID, *mut *mut c_void) -> i32,
    AddRef: extern "system" fn(*mut c_void) -> u32,
    Release: extern "system" fn(*mut c_void) -> u32,
    // IAudioRenderClient
    GetBuffer: extern "system" fn(*mut c_void, u32, *mut *mut c_void) -> i32,
    ReleaseBuffer: extern "system" fn(*mut c_void, u32, u32) -> i32,
}

#[repr(C)]
struct IAudioRenderClient { lpVtbl: *const IAudioRenderClientVtbl }

#[link(name = "ole32")]
extern "system" {
    fn CoInitializeEx(pvReserved: *mut c_void, dwCoInit: u32) -> i32;
    fn CoUninitialize();
    fn CoCreateInstance(rclsid: *const GUID, pUnkOuter: *mut c_void, dwClsCtx: u32, riid: *const GUID, ppv: *mut *mut c_void) -> i32;
    fn CoTaskMemFree(pv: *mut c_void);
}

#[link(name = "kernel32")]
extern "system" { fn Sleep(dwMilliseconds: u32); }

use crate::*;

const CLSCTX_ALL: u32 = 23; // inproc/server/handler
const COINIT_APARTMENTTHREADED: u32 = 0x2;

const ERender: u32 = 0;
const EConsole: u32 = 0;

const AUDCLNT_SHAREMODE_SHARED: u32 = 0;
const AUDCLNT_STREAMFLAGS_NONE: u32 = 0;

const CLSID_MMDeviceEnumerator: GUID = GUID { Data1: 0xBCDE0395, Data2: 0xE52F, Data3: 0x467C, Data4: [0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E] };
const IID_IMMDeviceEnumerator: GUID = GUID { Data1: 0xA95664D2, Data2: 0x9614, Data3: 0x4F35, Data4: [0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6] };
const IID_IAudioClient: GUID = GUID { Data1: 0x1CB9AD4C, Data2: 0xDBFA, Data3: 0x4C32, Data4: [0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2] };
const IID_IAudioRenderClient: GUID = GUID { Data1: 0xF294ACFC, Data2: 0x3146, Data3: 0x4483, Data4: [0xA7,0xBF,0xAD,0xDC,0xA7,0xC2,0x60,0xE2] };

pub fn impl_play_sine(freq_hz: f32, ms: u32) -> Result<(), AudioError> {
    unsafe {
        let hr = CoInitializeEx(std::ptr::null_mut(), COINIT_APARTMENTTHREADED);
        if hr < 0 { return Err(AudioError::Failed); }

        let mut p_enum: *mut c_void = std::ptr::null_mut();
        let hr = CoCreateInstance(&CLSID_MMDeviceEnumerator as *const GUID, std::ptr::null_mut(), CLSCTX_ALL, &IID_IMMDeviceEnumerator as *const GUID, &mut p_enum as *mut *mut c_void);
        if hr < 0 || p_enum.is_null() { CoUninitialize(); return Err(AudioError::Failed); }
        let enumr = p_enum as *mut IMMDeviceEnumerator;

        let mut p_dev: *mut c_void = std::ptr::null_mut();
        let hr = ((*(*enumr).lpVtbl).GetDefaultAudioEndpoint)(enumr as *mut c_void, ERender, EConsole, &mut p_dev as *mut *mut c_void);
        if hr < 0 || p_dev.is_null() { ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void); CoUninitialize(); return Err(AudioError::Failed); }
        let dev = p_dev as *mut IMMDevice;

        let mut p_client: *mut c_void = std::ptr::null_mut();
        let hr = ((*(*dev).lpVtbl).Activate)(dev as *mut c_void, &IID_IAudioClient as *const GUID, CLSCTX_ALL, std::ptr::null_mut(), &mut p_client as *mut *mut c_void);
        if hr < 0 || p_client.is_null() {
            ((*(*dev).lpVtbl).Release)(dev as *mut c_void);
            ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void);
            CoUninitialize();
            return Err(AudioError::Failed);
        }
        let client = p_client as *mut IAudioClient;

        let mut pwfx: *mut WAVEFORMATEX = std::ptr::null_mut();
        let hr = ((*(*client).lpVtbl).GetMixFormat)(client as *mut c_void, &mut pwfx as *mut *mut WAVEFORMATEX);
        if hr < 0 || pwfx.is_null() {
            ((*(*client).lpVtbl).Release)(client as *mut c_void);
            ((*(*dev).lpVtbl).Release)(dev as *mut c_void);
            ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void);
            CoUninitialize();
            return Err(AudioError::Failed);
        }

        let hns_duration: i64 = (ms as i64) * 10_000; // 1 ms = 10,000 100ns units
        let hr = ((*(*client).lpVtbl).Initialize)(client as *mut c_void, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NONE, hns_duration, 0, pwfx as *const WAVEFORMATEX, std::ptr::null());
        if hr < 0 {
            CoTaskMemFree(pwfx as *mut c_void);
            ((*(*client).lpVtbl).Release)(client as *mut c_void);
            ((*(*dev).lpVtbl).Release)(dev as *mut c_void);
            ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void);
            CoUninitialize();
            return Err(AudioError::Failed);
        }

        let mut frames: u32 = 0;
        let hr = ((*(*client).lpVtbl).GetBufferSize)(client as *mut c_void, &mut frames as *mut u32);
        if hr < 0 { CoTaskMemFree(pwfx as *mut c_void); ((*(*client).lpVtbl).Release)(client as *mut c_void); ((*(*dev).lpVtbl).Release)(dev as *mut c_void); ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void); CoUninitialize(); return Err(AudioError::Failed); }

        let mut p_render: *mut c_void = std::ptr::null_mut();
        let hr = ((*(*client).lpVtbl).GetService)(client as *mut c_void, &IID_IAudioRenderClient as *const GUID, &mut p_render as *mut *mut c_void);
        if hr < 0 || p_render.is_null() { CoTaskMemFree(pwfx as *mut c_void); ((*(*client).lpVtbl).Release)(client as *mut c_void); ((*(*dev).lpVtbl).Release)(dev as *mut c_void); ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void); CoUninitialize(); return Err(AudioError::Failed); }
        let render = p_render as *mut IAudioRenderClient;

        let mut p_data: *mut c_void = std::ptr::null_mut();
        let hr = ((*(*render).lpVtbl).GetBuffer)(render as *mut c_void, frames, &mut p_data as *mut *mut c_void);
        if hr < 0 || p_data.is_null() { ((*(*render).lpVtbl).Release)(render as *mut c_void); CoTaskMemFree(pwfx as *mut c_void); ((*(*client).lpVtbl).Release)(client as *mut c_void); ((*(*dev).lpVtbl).Release)(dev as *mut c_void); ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void); CoUninitialize(); return Err(AudioError::Failed); }

        let channels = (*pwfx).nChannels as usize;
        let sample_rate = (*pwfx).nSamplesPerSec as f32;
        let bits = (*pwfx).wBitsPerSample as usize;
        if (*pwfx).wFormatTag == WAVE_FORMAT_IEEE_FLOAT && bits == 32 {
            let buf = std::slice::from_raw_parts_mut(p_data as *mut f32, (frames as usize) * channels);
            for i in 0..(frames as usize) {
                let t = (i as f32) / sample_rate;
                let s = (2.0 * std::f32::consts::PI * freq_hz * t).sin() * 0.25;
                for ch in 0..channels { buf[i * channels + ch] = s; }
            }
        } else if (*pwfx).wFormatTag == WAVE_FORMAT_PCM && bits == 16 {
            let buf = std::slice::from_raw_parts_mut(p_data as *mut i16, (frames as usize) * channels);
            for i in 0..(frames as usize) {
                let t = (i as f32) / sample_rate;
                let s = (2.0 * std::f32::consts::PI * freq_hz * t).sin() * 0.25;
                let v = (s * 32767.0) as i16;
                for ch in 0..channels { buf[i * channels + ch] = v; }
            }
        } else {
            // unsupported format in this minimal sample
        }

        let _ = ((*(*render).lpVtbl).ReleaseBuffer)(render as *mut c_void, frames, 0);
        let _ = ((*(*client).lpVtbl).Start)(client as *mut c_void);
        Sleep(ms);
        let _ = ((*(*client).lpVtbl).Stop)(client as *mut c_void);

        CoTaskMemFree(pwfx as *mut c_void);
        ((*(*render).lpVtbl).Release)(render as *mut c_void);
        ((*(*client).lpVtbl).Release)(client as *mut c_void);
        ((*(*dev).lpVtbl).Release)(dev as *mut c_void);
        ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void);
        CoUninitialize();
        Ok(())
    }
}

pub fn impl_enumerate_devices(out: &mut [AudioDevice]) -> Result<usize, AudioError> {
    unsafe {
        let hr = CoInitializeEx(std::ptr::null_mut(), COINIT_APARTMENTTHREADED);
        if hr < 0 { return Err(AudioError::Failed); }
        let mut p_enum: *mut c_void = std::ptr::null_mut();
        let hr = CoCreateInstance(&CLSID_MMDeviceEnumerator as *const GUID, std::ptr::null_mut(), CLSCTX_ALL, &IID_IMMDeviceEnumerator as *const GUID, &mut p_enum as *mut *mut c_void);
        if hr < 0 || p_enum.is_null() { CoUninitialize(); return Err(AudioError::Failed); }
        let enumr = p_enum as *mut IMMDeviceEnumerator;
        let mut p_dev: *mut c_void = std::ptr::null_mut();
        let hr = ((*(*enumr).lpVtbl).GetDefaultAudioEndpoint)(enumr as *mut c_void, ERender, EConsole, &mut p_dev as *mut *mut c_void);
        if hr < 0 || p_dev.is_null() { ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void); CoUninitialize(); return Err(AudioError::Failed); }
        let dev = p_dev as *mut IMMDevice;
        let mut p_client: *mut c_void = std::ptr::null_mut();
        let hr = ((*(*dev).lpVtbl).Activate)(dev as *mut c_void, &IID_IAudioClient as *const GUID, CLSCTX_ALL, std::ptr::null_mut(), &mut p_client as *mut *mut c_void);
        if hr < 0 || p_client.is_null() { ((*(*dev).lpVtbl).Release)(dev as *mut c_void); ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void); CoUninitialize(); return Err(AudioError::Failed); }
        let client = p_client as *mut IAudioClient;
        let mut pwfx: *mut WAVEFORMATEX = std::ptr::null_mut();
        let hr = ((*(*client).lpVtbl).GetMixFormat)(client as *mut c_void, &mut pwfx as *mut *mut WAVEFORMATEX);
        if hr < 0 || pwfx.is_null() { ((*(*client).lpVtbl).Release)(client as *mut c_void); ((*(*dev).lpVtbl).Release)(dev as *mut c_void); ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void); CoUninitialize(); return Err(AudioError::Failed); }
        let mut count = 0usize;
        if !out.is_empty() {
            out[0] = AudioDevice { sample_rate: (*pwfx).nSamplesPerSec, channels: (*pwfx).nChannels };
            count = 1;
        }
        CoTaskMemFree(pwfx as *mut c_void);
        ((*(*client).lpVtbl).Release)(client as *mut c_void);
        ((*(*dev).lpVtbl).Release)(dev as *mut c_void);
        ((*(*enumr).lpVtbl).Release)(enumr as *mut c_void);
        CoUninitialize();
        Ok(count)
    }
}
