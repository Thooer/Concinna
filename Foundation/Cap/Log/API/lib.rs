use cap_stream::{FileWriter, StreamError};
use prm_time::{now};

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LogLevel { Trace, Debug, Info, Warn, Error }

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum LogError { Failed, BufferTooSmall }

pub struct Logger { writer: FileWriter, level: LogLevel }

impl Logger {
    pub fn create(path: &str, level: LogLevel) -> Result<Self, LogError> {
        match FileWriter::create(path) { Ok(w) => Ok(Self { writer: w, level }), Err(_) => Err(LogError::Failed) }
    }
    pub fn set_level(&mut self, level: LogLevel) { self.level = level }
    fn level_enabled(&self, lv: LogLevel) -> bool {
        match (self.level, lv) {
            (LogLevel::Trace, _) => true,
            (LogLevel::Debug, LogLevel::Trace) => false,
            (LogLevel::Debug, _) => true,
            (LogLevel::Info, LogLevel::Trace) | (LogLevel::Info, LogLevel::Debug) => false,
            (LogLevel::Info, _) => true,
            (LogLevel::Warn, LogLevel::Error) | (LogLevel::Warn, LogLevel::Warn) => true,
            (LogLevel::Warn, _) => false,
            (LogLevel::Error, LogLevel::Error) => true,
            (LogLevel::Error, _) => false,
        }
    }
    pub fn log(&mut self, lv: LogLevel, msg: &str) -> Result<(), LogError> {
        if !self.level_enabled(lv) { return Ok(()); }
        let ts = now();
        let mut buf = [0u8; 1024];
        let mut o = 0usize;
        o += write_int(&mut buf[o..], ts).map_err(|_| LogError::BufferTooSmall)?;
        o += write_str(&mut buf[o..], " ").map_err(|_| LogError::BufferTooSmall)?;
        o += write_str(&mut buf[o..], level_str(lv)).map_err(|_| LogError::BufferTooSmall)?;
        o += write_str(&mut buf[o..], " ").map_err(|_| LogError::BufferTooSmall)?;
        o += write_str(&mut buf[o..], msg).map_err(|_| LogError::BufferTooSmall)?;
        o += write_str(&mut buf[o..], "\n").map_err(|_| LogError::BufferTooSmall)?;
        match self.writer.write(&buf[..o]) { Ok(_) => Ok(()), Err(_) => Err(LogError::Failed) }
    }
    pub fn flush(&mut self) -> Result<(), LogError> { self.writer.flush().map_err(|_| LogError::Failed) }
}

fn level_str(lv: LogLevel) -> &'static str {
    match lv { LogLevel::Trace => "TRACE", LogLevel::Debug => "DEBUG", LogLevel::Info => "INFO", LogLevel::Warn => "WARN", LogLevel::Error => "ERROR" }
}

fn write_str(out: &mut [u8], s: &str) -> Result<usize, ()> {
    let bs = s.as_bytes(); if out.len() < bs.len() { return Err(()); }
    for i in 0..bs.len() { out[i] = bs[i]; }
    Ok(bs.len())
}

fn write_int(out: &mut [u8], v: i64) -> Result<usize, ()> {
    let mut tmp = [0u8; 32]; let mut n = 0usize; let mut x = if v < 0 { -(v as i128) as u64 } else { v as u64 };
    if v < 0 { if out.len() == 0 { return Err(()); } out[0] = b'-'; }
    if x == 0 { if out.len() < 1 + (v < 0) as usize { return Err(()); } let o = (v < 0) as usize; out[o] = b'0'; return Ok(o+1); }
    while x > 0 { let d = (x % 10) as u8; tmp[n] = b'0' + d; n += 1; x /= 10; }
    let mut o = 0usize; if v < 0 { out[o] = b'-'; o += 1; }
    if out.len() < o + n { return Err(()); }
    for i in 0..n { out[o+i] = tmp[n-1-i]; }
    Ok(o+n)
}
