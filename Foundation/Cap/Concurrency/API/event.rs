use prm_threading::{EventHandle, ThreadingError};

pub struct Event { h: EventHandle }

impl Event {
    pub fn new(manual_reset: bool, initial_state: bool) -> Result<Self, ThreadingError> { Ok(Self { h: prm_threading::event_create(manual_reset, initial_state)? }) }
    pub fn wait(&self, timeout_ms: u32) -> Result<(), ThreadingError> { prm_threading::event_wait(self.h, timeout_ms) }
    pub fn signal(&self) -> Result<(), ThreadingError> { prm_threading::event_signal(self.h) }
    pub fn reset(&self) -> Result<(), ThreadingError> { prm_threading::event_reset(self.h) }
    pub fn handle(&self) -> EventHandle { self.h }
}

impl Drop for Event { fn drop(&mut self) { let _ = prm_threading::event_destroy(self.h); } }
