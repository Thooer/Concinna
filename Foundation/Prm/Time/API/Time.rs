pub type TimePoint = i64;
pub type Duration = i64;

pub trait TimeSource {
    fn now(&self) -> TimePoint;
}

pub struct MonotonicSource;

impl TimeSource for MonotonicSource {
    fn now(&self) -> TimePoint { now() }
}

pub fn init() {}

pub fn default_source() -> MonotonicSource { MonotonicSource }

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Time.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    pub fn impl_now_ns() -> i64 { 0 }
    pub fn impl_sleep_ms(_ms: u32) {}
    pub fn impl_sleep_precise_ns(_ns: i64) {}
}

pub fn now() -> TimePoint { backend::impl_now_ns() }

pub fn sleep_ms(ms: u32) { backend::impl_sleep_ms(ms) }

pub fn sleep_precise_ns(ns: i64) { backend::impl_sleep_precise_ns(ns) }

pub fn delta(start: TimePoint, end: TimePoint) -> Duration { end - start }

pub fn delta_seconds(start: TimePoint, end: TimePoint) -> f64 { to_seconds(delta(start, end)) }

pub fn to_seconds(d: Duration) -> f64 { d as f64 / 1_000_000_000.0 }

pub fn to_milliseconds(d: Duration) -> f64 { d as f64 / 1_000_000.0 }
