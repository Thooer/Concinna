use prm_threading::{MutexHandle, ThreadingError};

pub struct Mutex { h: MutexHandle }

impl Mutex {
    pub fn new() -> Result<Self, ThreadingError> { Ok(Self { h: prm_threading::mutex_create()? }) }
    pub fn lock(&self) -> Result<(), ThreadingError> { prm_threading::mutex_lock(self.h) }
    pub fn unlock(&self) -> Result<(), ThreadingError> { prm_threading::mutex_unlock(self.h) }
    pub fn try_lock(&self) -> Result<bool, ThreadingError> { prm_threading::mutex_try_lock(self.h) }
    pub fn handle(&self) -> MutexHandle { self.h }
}

impl Drop for Mutex { fn drop(&mut self) { let _ = prm_threading::mutex_destroy(self.h); } }
