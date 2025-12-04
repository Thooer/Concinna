use std::alloc::{alloc, dealloc, Layout};
use crate::types::*;
use crate::resource::*;
use std::sync::atomic::{AtomicPtr, AtomicUsize, Ordering};

impl IMemoryResource for SystemMemoryResource {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        if !Alignment::is_power_of_two(align) { return Err(MemoryError::InvalidArgument); }
        let layout = match Layout::from_size_align(size, align) { Ok(l) => l, Err(_) => return Err(MemoryError::InvalidArgument) };
        unsafe {
            let p = alloc(layout);
            if p.is_null() { Err(MemoryError::OutOfMemory) } else { Ok(MemoryBlock::new(p, size)) }
        }
    }
    fn deallocate(&mut self, block: MemoryBlock, align: usize) {
        if block.is_empty() { return; }
        if !Alignment::is_power_of_two(align) { return; }
        let layout = match Layout::from_size_align(block.size, align) { Ok(l) => l, Err(_) => return };
        unsafe { dealloc(block.ptr, layout); }
    }
    fn reallocate(&mut self, block: MemoryBlock, new_size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if new_size == 0 { if !block.is_empty() { self.deallocate(block, align); } return Ok(MemoryBlock::empty()); }
        let nb = self.allocate(new_size, align)?;
        if !block.is_empty() {
            unsafe {
                let n = core::cmp::min(block.size, new_size);
                core::ptr::copy_nonoverlapping(block.ptr, nb.ptr, n);
            }
            self.deallocate(block, align);
        }
        Ok(nb)
    }
    fn is_equal(&self, other: &dyn IMemoryResource) -> bool { core::ptr::eq(self as &dyn IMemoryResource, other) }
    fn reset(&mut self) {}
}

impl FrameAllocatorResource {
    pub fn new(capacity: usize) -> Self {
        let cap = capacity;
        let layout = Layout::from_size_align(cap, Alignment::DEFAULT).unwrap_or_else(|_| Layout::from_size_align(Alignment::DEFAULT, Alignment::DEFAULT).unwrap());
        let base = unsafe { alloc(layout) };
        Self { base, capacity: cap, committed: cap, offset: 0 }
    }
    pub fn offset(&self) -> usize { self.offset }
    pub fn reset_to(&mut self, off: usize) { self.offset = off.min(self.capacity) }
}

impl Drop for FrameAllocatorResource {
    fn drop(&mut self) {
        if !self.base.is_null() {
            let layout = Layout::from_size_align(self.capacity, Alignment::DEFAULT).ok();
            if let Some(l) = layout { unsafe { dealloc(self.base, l); } }
        }
    }
}

impl IMemoryResource for FrameAllocatorResource {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        if !Alignment::is_power_of_two(align) { return Err(MemoryError::InvalidArgument); }
        let start = Alignment::align_up(self.offset, align);
        let end = start.checked_add(size).ok_or(MemoryError::Failed)?;
        if end > self.capacity { return Err(MemoryError::OutOfMemory); }
        self.offset = end;
        unsafe { Ok(MemoryBlock::new(self.base.add(start), size)) }
    }
    fn deallocate(&mut self, _block: MemoryBlock, _align: usize) {}
    fn reallocate(&mut self, block: MemoryBlock, new_size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if new_size == 0 { return Ok(MemoryBlock::empty()); }
        let nb = self.allocate(new_size, align)?;
        if !block.is_empty() {
            unsafe {
                let n = core::cmp::min(block.size, new_size);
                core::ptr::copy_nonoverlapping(block.ptr, nb.ptr, n);
            }
        }
        Ok(nb)
    }
    fn is_equal(&self, other: &dyn IMemoryResource) -> bool { core::ptr::eq(self as &dyn IMemoryResource, other) }
    fn reset(&mut self) { self.offset = 0; }
}

impl StackAllocatorResource {
    pub fn new(capacity: usize) -> Self {
        let cap = capacity;
        let layout = Layout::from_size_align(cap, Alignment::DEFAULT).unwrap_or_else(|_| Layout::from_size_align(Alignment::DEFAULT, Alignment::DEFAULT).unwrap());
        let base = unsafe { alloc(layout) };
        Self { base, capacity: cap, committed: cap, offset: 0 }
    }
    pub fn marker(&self) -> usize { self.offset }
    pub fn reset_to(&mut self, off: usize) { self.offset = off.min(self.capacity) }
}

impl Drop for StackAllocatorResource {
    fn drop(&mut self) {
        if !self.base.is_null() {
            let layout = Layout::from_size_align(self.capacity, Alignment::DEFAULT).ok();
            if let Some(l) = layout { unsafe { dealloc(self.base, l); } }
        }
    }
}

impl IMemoryResource for StackAllocatorResource {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        if !Alignment::is_power_of_two(align) { return Err(MemoryError::InvalidArgument); }
        let start = Alignment::align_up(self.offset, align);
        let end = start.checked_add(size).ok_or(MemoryError::Failed)?;
        if end > self.capacity { return Err(MemoryError::OutOfMemory); }
        self.offset = end;
        unsafe { Ok(MemoryBlock::new(self.base.add(start), size)) }
    }
    fn deallocate(&mut self, block: MemoryBlock, _align: usize) {
        if block.is_empty() { return; }
        let base = self.base as usize;
        let end = base + self.offset;
        let blk_end = block.ptr as usize + block.size;
        if blk_end == end {
            let new_off = (block.ptr as usize).saturating_sub(base);
            self.offset = new_off;
        }
    }
    fn reallocate(&mut self, block: MemoryBlock, new_size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if new_size == 0 { return Ok(MemoryBlock::empty()); }
        let nb = self.allocate(new_size, align)?;
        if !block.is_empty() {
            unsafe {
                let n = core::cmp::min(block.size, new_size);
                core::ptr::copy_nonoverlapping(block.ptr, nb.ptr, n);
            }
            self.deallocate(block, align);
        }
        Ok(nb)
    }
    fn is_equal(&self, other: &dyn IMemoryResource) -> bool { core::ptr::eq(self as &dyn IMemoryResource, other) }
    fn reset(&mut self) { self.offset = 0; }
}

impl PoolAllocatorResource {
    pub fn new(block_size: usize, capacity: usize, alignment: usize) -> Self {
        let bs = core::cmp::max(block_size, 1);
        let al = if Alignment::is_power_of_two(alignment) { alignment } else { Alignment::DEFAULT };
        let cap = capacity;
        let layout = Layout::from_size_align(cap, Alignment::DEFAULT).unwrap_or_else(|_| Layout::from_size_align(Alignment::DEFAULT, Alignment::DEFAULT).unwrap());
        let base = unsafe { alloc(layout) };
        Self { base, capacity: cap, committed: AtomicUsize::new(cap), block_size: bs, alignment: al, owner_thread_id: 0, remote_free_head: AtomicPtr::new(core::ptr::null_mut()) }
    }
}

const SIZE_CLASSES: [usize; crate::resource::SMALL_OBJECT_CLASS_COUNT] = [16, 32, 64, 128, 256, 512];

impl SmallObjectAllocatorResource {
    pub fn new(total_capacity: usize) -> Self {
        use core::mem::MaybeUninit;
        let mut pools_uninit: [MaybeUninit<PoolAllocatorResource>; SIZE_CLASSES.len()] = unsafe { MaybeUninit::uninit().assume_init() };
        let per_pool = total_capacity / SIZE_CLASSES.len().max(1);
        for (i, bs) in SIZE_CLASSES.iter().enumerate() { pools_uninit[i].write(PoolAllocatorResource::new(*bs, per_pool, Alignment::DEFAULT)); }
        let pools: [PoolAllocatorResource; SIZE_CLASSES.len()] = unsafe { core::mem::transmute_copy(&pools_uninit) };
        core::mem::forget(pools_uninit);
        SmallObjectAllocatorResource { pools }
    }
}

impl IMemoryResource for SmallObjectAllocatorResource {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        if !Alignment::is_power_of_two(align) { return Err(MemoryError::InvalidArgument); }
        for (i, bs) in SIZE_CLASSES.iter().enumerate() { if size <= *bs && align <= *bs { return self.pools[i].allocate(*bs, *bs); } }
        Err(MemoryError::NotSupported)
    }
    fn deallocate(&mut self, block: MemoryBlock, align: usize) {
        if block.is_empty() { return; }
        for (i, bs) in SIZE_CLASSES.iter().enumerate() { if block.size == *bs && align <= *bs { self.pools[i].deallocate(block, *bs); return; } }
    }
    fn reallocate(&mut self, block: MemoryBlock, new_size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if new_size == 0 { if !block.is_empty() { self.deallocate(block, align); } return Ok(MemoryBlock::empty()); }
        for bs in SIZE_CLASSES.iter() { if block.size == *bs && new_size <= *bs && align <= *bs { return Ok(block); } }
        let nb = self.allocate(new_size, align)?;
        unsafe { core::ptr::copy_nonoverlapping(block.ptr, nb.ptr, core::cmp::min(block.size, nb.size)); }
        self.deallocate(block, align);
        Ok(nb)
    }
    fn is_equal(&self, other: &dyn IMemoryResource) -> bool { core::ptr::eq(self as &dyn IMemoryResource, other) }
    fn reset(&mut self) { for p in self.pools.iter_mut() { p.reset(); } }
}

impl<'a> TaggedAllocatorResource<'a> {
    pub fn new(inner: &'a mut dyn IMemoryResource, tag_id: usize, limit: usize) -> Self {
        TaggedAllocatorResource { inner: inner as *mut (dyn IMemoryResource + 'a), tag_id, limit, used: AtomicUsize::new(0) }
    }
}

impl<'a> IMemoryResource for TaggedAllocatorResource<'a> {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        let cur = self.used.load(Ordering::Acquire);
        if cur.checked_add(size).map(|v| v <= self.limit).unwrap_or(false) {
            let mb = unsafe { (&mut *self.inner).allocate(size, align)? };
            self.used.store(cur + mb.size, Ordering::Release);
            Ok(mb)
        } else { Err(MemoryError::OutOfMemory) }
    }
    fn deallocate(&mut self, block: MemoryBlock, align: usize) {
        if block.is_empty() { return; }
        unsafe { (&mut *self.inner).deallocate(block, align); }
        let cur = self.used.load(Ordering::Acquire);
        self.used.store(cur.saturating_sub(block.size), Ordering::Release);
    }
    fn reallocate(&mut self, block: MemoryBlock, new_size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if new_size == 0 { if !block.is_empty() { self.deallocate(block, align); } return Ok(MemoryBlock::empty()); }
        let cur = self.used.load(Ordering::Acquire);
        let new_used = cur.saturating_sub(block.size).checked_add(new_size).ok_or(MemoryError::Failed)?;
        if new_used <= self.limit {
            let nb = unsafe { (&mut *self.inner).reallocate(block, new_size, align)? };
            self.used.store(new_used, Ordering::Release);
            Ok(nb)
        } else { Err(MemoryError::OutOfMemory) }
    }
    fn is_equal(&self, other: &dyn IMemoryResource) -> bool { core::ptr::eq(self as &dyn IMemoryResource, other) }
    fn reset(&mut self) { self.used.store(0, Ordering::Release); }
}

impl Drop for PoolAllocatorResource {
    fn drop(&mut self) {
        if !self.base.is_null() {
            let layout = Layout::from_size_align(self.capacity, Alignment::DEFAULT).ok();
            if let Some(l) = layout { unsafe { dealloc(self.base, l); } }
        }
    }
}

impl IMemoryResource for PoolAllocatorResource {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        if !Alignment::is_power_of_two(align) { return Err(MemoryError::InvalidArgument); }
        if size > self.block_size { return Err(MemoryError::NotSupported); }
        // remote free queue pop
        loop {
            let head = self.remote_free_head.load(Ordering::Acquire);
            if head.is_null() { break; }
            unsafe {
                let next = *(head as *mut *mut u8);
                if self.remote_free_head.compare_exchange(head, next, Ordering::AcqRel, Ordering::Relax).is_ok() {
                    return Ok(MemoryBlock::new(head, self.block_size));
                }
            }
        }
        // fallback: bump within reserved buffer
        let cur = self.committed.load(Ordering::Acquire);
        let start = Alignment::align_up(cur, self.block_size.max(align));
        let end = start.checked_add(self.block_size).ok_or(MemoryError::Failed)?;
        if end > self.capacity { return Err(MemoryError::OutOfMemory); }
        self.committed.store(end, Ordering::Release);
        unsafe { Ok(MemoryBlock::new(self.base.add(start), self.block_size)) }
    }
    fn deallocate(&mut self, block: MemoryBlock, _align: usize) {
        if block.is_empty() { return; }
        // push to remote free list (lock-free)
        loop {
            let head = self.remote_free_head.load(Ordering::Acquire);
            unsafe { *(block.ptr as *mut *mut u8) = head; }
            if self.remote_free_head.compare_exchange(head, block.ptr, Ordering::AcqRel, Ordering::Relax).is_ok() { break; }
        }
    }
    fn reallocate(&mut self, block: MemoryBlock, new_size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if new_size == 0 { if !block.is_empty() { self.deallocate(block, align); } return Ok(MemoryBlock::empty()); }
        if new_size > self.block_size { return Err(MemoryError::NotSupported); }
        Ok(block)
    }
    fn is_equal(&self, other: &dyn IMemoryResource) -> bool { core::ptr::eq(self as &dyn IMemoryResource, other) }
    fn reset(&mut self) { self.remote_free_head.store(core::ptr::null_mut(), Ordering::Release); }
}
