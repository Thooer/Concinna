use crate::types::*;

pub trait IMemoryResource {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError>;
    fn deallocate(&mut self, block: MemoryBlock, align: usize);
    fn reallocate(&mut self, block: MemoryBlock, new_size: usize, align: usize) -> Result<MemoryBlock, MemoryError>;
    fn is_equal(&self, other: &dyn IMemoryResource) -> bool;
    fn reset(&mut self);
}

pub struct SystemMemoryResource;

pub struct FrameAllocatorResource { pub base: *mut u8, pub capacity: usize, pub committed: usize, pub offset: usize }

pub struct StackAllocatorResource { pub base: *mut u8, pub capacity: usize, pub committed: usize, pub offset: usize }

use std::sync::atomic::{AtomicUsize, AtomicPtr};

pub struct PoolAllocatorResource { pub base: *mut u8, pub capacity: usize, pub committed: AtomicUsize, pub block_size: usize, pub alignment: usize, pub owner_thread_id: usize, pub remote_free_head: AtomicPtr<u8> }

pub const SMALL_OBJECT_CLASS_COUNT: usize = 6;
pub struct SmallObjectAllocatorResource { pub pools: [PoolAllocatorResource; SMALL_OBJECT_CLASS_COUNT] }

pub struct TaggedAllocatorResource<'a> { pub inner: *mut (dyn IMemoryResource + 'a), pub tag_id: usize, pub limit: usize, pub used: AtomicUsize }
