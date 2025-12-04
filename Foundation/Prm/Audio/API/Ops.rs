mod types;
pub use types::*;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Audio.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
    pub fn impl_play_sine(_freq_hz: f32, _ms: u32) -> Result<(), AudioError> { Err(AudioError::Unsupported) }
    pub fn impl_enumerate_devices(_out: &mut [AudioDevice]) -> Result<usize, AudioError> { Err(AudioError::Unsupported) }
}

pub fn play_sine(freq_hz: f32, ms: u32) -> Result<(), AudioError> { backend::impl_play_sine(freq_hz, ms) }
pub fn enumerate_devices(out: &mut [AudioDevice]) -> Result<usize, AudioError> { backend::impl_enumerate_devices(out) }
