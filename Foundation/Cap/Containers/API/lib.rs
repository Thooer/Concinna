#![no_std]
pub mod vector;
pub mod mpmc_queue;
pub mod deque;
pub mod chase_lev_deque;
pub mod ring_buffer;
pub mod string;
pub mod hash_map;
pub use vector::*;
pub use mpmc_queue::*;
pub use deque::*;
pub use chase_lev_deque::*;
pub use ring_buffer::*;
pub use string::*;
pub use hash_map::*;

use cap_memory::{Allocator, MemoryError};

// --- SoA core protocols and modes ---

// Column protocol: any container that wants to be used as an SoA column
pub trait SoAColumn<'a, T>: Sized {
    fn with_capacity(alloc: Allocator<'a>, capacity: usize) -> Result<Self, MemoryError>;
    fn with_capacity_aligned(alloc: Allocator<'a>, capacity: usize, align: usize) -> Result<Self, MemoryError>;
    fn push(&mut self, value: T) -> Result<(), MemoryError>;
    fn len(&self) -> usize;
    fn get(&self, index: usize) -> Option<T>;
    fn as_ptr(&self) -> *const T;
}

// Mapping protocol: defines how a type T is stored under strategy M
pub trait SoA<M> {
    type Storage<'a>: SoAColumn<'a, Self>
    where
        Self: Sized;
}

// Strategy tags
pub struct UseVector;
pub struct UseDeque;

// Primitive mappings decided by the container library
impl SoA<UseVector> for f32 {
    type Storage<'a> = crate::Vector<'a, f32>;
}

impl SoA<UseVector> for u32 {
    type Storage<'a> = crate::Vector<'a, u32>;
}

impl SoA<UseDeque> for f32 {
    type Storage<'a> = crate::Deque<'a, f32>;
}

impl SoA<UseDeque> for u32 {
    type Storage<'a> = crate::Deque<'a, u32>;
}

// Column implementations for containers
impl<'a, T: Copy> SoAColumn<'a, T> for crate::Vector<'a, T> {
    fn with_capacity(alloc: Allocator<'a>, capacity: usize) -> Result<Self, MemoryError> {
        Self::with_capacity(alloc, capacity)
    }
    fn with_capacity_aligned(alloc: Allocator<'a>, capacity: usize, align: usize) -> Result<Self, MemoryError> {
        Self::with_capacity_aligned(alloc, capacity, align)
    }
    fn push(&mut self, value: T) -> Result<(), MemoryError> {
        Self::push(self, value)
    }
    fn len(&self) -> usize { Self::len(self) }
    fn get(&self, index: usize) -> Option<T> { Self::get(self, index) }
    fn as_ptr(&self) -> *const T { Self::as_ptr(self) }
}

impl<'a, T: Copy> SoAColumn<'a, T> for crate::Deque<'a, T> {
    fn with_capacity(alloc: Allocator<'a>, capacity: usize) -> Result<Self, MemoryError> {
        let cap_pow2 = capacity.next_power_of_two();
        if cap_pow2 == 0 { return Err(MemoryError::InvalidArgument); }
        Self::with_capacity(alloc, cap_pow2)
    }
    fn with_capacity_aligned(alloc: Allocator<'a>, capacity: usize, _align: usize) -> Result<Self, MemoryError> {
        // Deque uses power-of-two capacity; alignment not configurable
        let cap_pow2 = capacity.next_power_of_two();
        if cap_pow2 == 0 { return Err(MemoryError::InvalidArgument); }
        Self::with_capacity(alloc, cap_pow2)
    }
    fn push(&mut self, value: T) -> Result<(), MemoryError> { self.push_back(value) }
    fn len(&self) -> usize { self.len() }
    fn get(&self, index: usize) -> Option<T> { self.get(index) }
    fn as_ptr(&self) -> *const T { core::ptr::null() }
}
