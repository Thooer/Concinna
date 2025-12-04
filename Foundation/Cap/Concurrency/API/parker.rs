use prm_threading::{EventHandle, ThreadingError};

pub struct Parker { h: EventHandle }

impl Parker {
    pub fn new() -> Result<Self, ThreadingError> { Ok(Self { h: prm_threading::event_create(false, false)? }) }
    pub fn park(&self, timeout_ms: u32) -> Result<(), ThreadingError> { prm_threading::event_wait(self.h, timeout_ms) }
    pub fn unpark(&self) -> Result<(), ThreadingError> { prm_threading::event_signal(self.h) }
    pub fn reset(&self) -> Result<(), ThreadingError> { prm_threading::event_reset(self.h) }
    pub fn handle(&self) -> EventHandle { self.h }
}

impl Drop for Parker { fn drop(&mut self) { let _ = prm_threading::event_destroy(self.h); } }
