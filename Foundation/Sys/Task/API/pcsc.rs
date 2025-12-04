use core::ffi::c_void;
use cap_memory::{Allocator, MemoryBlock, MemoryError};

#[derive(Clone, Copy)]
pub struct Command { pub exec: fn(*mut c_void), pub ctx: *mut c_void }

pub struct CommandBuffer<'a> { ptr: *mut Command, cap: usize, len: usize, blk: MemoryBlock, alloc: Allocator<'a> }

impl<'a> CommandBuffer<'a> {
    pub fn new(mut alloc: Allocator<'a>, cap: usize) -> Result<Self, MemoryError> {
        let bytes = cap.checked_mul(core::mem::size_of::<Command>()).ok_or(MemoryError::Failed)?;
        let blk = alloc.alloc(bytes, core::mem::align_of::<Command>())?;
        Ok(Self { ptr: blk.ptr.cast::<Command>(), cap, len: 0, blk, alloc })
    }
    pub fn push(&mut self, exec: fn(*mut c_void), ctx: *mut c_void) -> bool {
        if self.len >= self.cap { return false; }
        unsafe { self.ptr.add(self.len).write(Command { exec, ctx }); }
        self.len += 1;
        true
    }
    pub fn clear(&mut self) { self.len = 0; }
    pub fn apply_all(&mut self) { unsafe { for i in 0..self.len { let c = self.ptr.add(i).read(); (c.exec)(c.ctx); } } self.clear(); }
}

impl<'a> Drop for CommandBuffer<'a> { fn drop(&mut self) { if !self.blk.is_empty() { let _ = self.alloc.free(self.blk, core::mem::align_of::<Command>()); } } }

pub struct CommandAggregator<'a> { ptr: *mut *mut CommandBuffer<'a>, cap: usize, len: usize, blk: MemoryBlock, alloc: Allocator<'a> }

impl<'a> CommandAggregator<'a> {
    pub fn new(mut alloc: Allocator<'a>, cap: usize) -> Result<Self, MemoryError> {
        let bytes = cap.checked_mul(core::mem::size_of::<*mut CommandBuffer<'a>>()).ok_or(MemoryError::Failed)?;
        let blk = alloc.alloc(bytes, core::mem::align_of::<*mut CommandBuffer<'a>>())?;
        Ok(Self { ptr: blk.ptr.cast::<*mut CommandBuffer<'a>>(), cap, len: 0, blk, alloc })
    }
    pub fn add(&mut self, buf: &mut CommandBuffer<'a>) -> bool { if self.len >= self.cap { return false; } unsafe { self.ptr.add(self.len).write(buf as *mut _); } self.len += 1; true }
    pub fn apply_all(&mut self) { unsafe { for i in 0..self.len { let b = &mut *self.ptr.add(i).read(); b.apply_all(); } } }
}

impl<'a> Drop for CommandAggregator<'a> { fn drop(&mut self) { if !self.blk.is_empty() { let _ = self.alloc.free(self.blk, core::mem::align_of::<*mut CommandBuffer<'a>>()); } } }
