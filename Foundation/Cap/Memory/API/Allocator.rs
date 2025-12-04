use crate::types::*;
use crate::resource::*;
#[derive(Clone, Copy)]
pub struct Allocator<'a> { pub resource: *mut (dyn IMemoryResource + 'a) }

impl<'a> Allocator<'a> {
    pub fn new(resource: &'a mut dyn IMemoryResource) -> Self { Self { resource: resource as *mut (dyn IMemoryResource + 'a) } }
    pub unsafe fn from_raw(ptr: *mut (dyn IMemoryResource + 'a)) -> Self { Self { resource: ptr } }
    pub fn alloc(&self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> { unsafe { (&mut *self.resource).allocate(size, align) } }
    pub fn free(&self, block: MemoryBlock, align: usize) { unsafe { (&mut *self.resource).deallocate(block, align) } }
    pub fn realloc(&self, block: MemoryBlock, new_size: usize, align: usize) -> Result<MemoryBlock, MemoryError> { unsafe { (&mut *self.resource).reallocate(block, new_size, align) } }
    pub fn reset(&self) { unsafe { (&mut *self.resource).reset() } }
}
