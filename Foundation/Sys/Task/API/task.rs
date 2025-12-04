use core::ffi::c_void;

#[derive(Clone, Copy)]
pub struct TaskHandle { pub id: usize }

pub type TaskFunc = fn(*mut c_void);

#[derive(Clone, Copy)]
pub enum ThreadAffinity { Main, Any, Compute(u16) }

#[derive(Clone, Copy)]
pub enum Trigger { NextTask(usize), NextFrameRoot(usize) }
