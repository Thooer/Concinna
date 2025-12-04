#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AudioError { Failed, Unsupported }

#[derive(Clone, Copy)]
pub struct AudioDevice { pub sample_rate: u32, pub channels: u16 }
