use crate::types::*;
use crate::resource::*;
use std::sync::atomic::{AtomicPtr, AtomicUsize, Ordering};
use core::ffi::c_void;

const MEM_COMMIT: u32 = 0x00001000;
const MEM_RESERVE: u32 = 0x00002000;
const MEM_DECOMMIT: u32 = 0x00004000;
const MEM_RELEASE: u32 = 0x00008000;
const PAGE_READWRITE: u32 = 0x04;

#[repr(C)]
struct SYSTEM_INFO {
    wProcessorArchitecture: u16,
    wReserved: u16,
    dwPageSize: u32,
    lpMinimumApplicationAddress: *mut c_void,
    lpMaximumApplicationAddress: *mut c_void,
    dwActiveProcessorMask: usize,
    dwNumberOfProcessors: u32,
    dwProcessorType: u32,
    dwAllocationGranularity: u32,
    wProcessorLevel: u16,
    wProcessorRevision: u16,
}

extern "system" {
    fn VirtualAlloc(lpAddress: *mut c_void, dwSize: usize, flAllocationType: u32, flProtect: u32) -> *mut c_void;
    fn VirtualFree(lpAddress: *mut c_void, dwSize: usize, dwFreeType: u32) -> i32;
    fn GetSystemInfo(lpSystemInfo: *mut SYSTEM_INFO);
    fn GetCurrentThreadId() -> u32;
}

fn page_size() -> usize {
    let mut info = SYSTEM_INFO {
        wProcessorArchitecture: 0,
        wReserved: 0,
        dwPageSize: 0,
        lpMinimumApplicationAddress: core::ptr::null_mut(),
        lpMaximumApplicationAddress: core::ptr::null_mut(),
        dwActiveProcessorMask: 0,
        dwNumberOfProcessors: 0,
        dwProcessorType: 0,
        dwAllocationGranularity: 0,
        wProcessorLevel: 0,
        wProcessorRevision: 0,
    };
    unsafe { GetSystemInfo(&mut info as *mut SYSTEM_INFO) };
    info.dwPageSize as usize
}

fn allocation_granularity() -> usize {
    let mut info = SYSTEM_INFO {
        wProcessorArchitecture: 0,
        wReserved: 0,
        dwPageSize: 0,
        lpMinimumApplicationAddress: core::ptr::null_mut(),
        lpMaximumApplicationAddress: core::ptr::null_mut(),
        dwActiveProcessorMask: 0,
        dwNumberOfProcessors: 0,
        dwProcessorType: 0,
        dwAllocationGranularity: 0,
        wProcessorLevel: 0,
        wProcessorRevision: 0,
    };
    unsafe { GetSystemInfo(&mut info as *mut SYSTEM_INFO) };
    info.dwAllocationGranularity as usize
}

unsafe fn vm_reserve(size: usize) -> *mut u8 { VirtualAlloc(core::ptr::null_mut(), size, MEM_RESERVE, PAGE_READWRITE) as *mut u8 }
unsafe fn vm_commit(base: *mut u8, size: usize) -> bool { !VirtualAlloc(base as *mut c_void, size, MEM_COMMIT, PAGE_READWRITE).is_null() }
unsafe fn vm_decommit(base: *mut u8, size: usize) -> bool { VirtualFree(base as *mut c_void, size, MEM_DECOMMIT) != 0 }
unsafe fn vm_release(base: *mut u8) -> bool { VirtualFree(base as *mut c_void, 0, MEM_RELEASE) != 0 }

impl SystemMemoryResource {
    fn alloc_pages(&mut self, size: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        let aligned = Alignment::align_up(size, page_size());
        let p = unsafe { VirtualAlloc(core::ptr::null_mut(), aligned, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE) } as *mut u8;
        if p.is_null() { Err(MemoryError::OutOfMemory) } else { Ok(MemoryBlock::new(p, size)) }
    }
}

impl IMemoryResource for SystemMemoryResource {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        if !Alignment::is_power_of_two(align) { return Err(MemoryError::InvalidArgument); }
        let ps = page_size();
        if align > ps { return Err(MemoryError::InvalidArgument); }
        self.alloc_pages(size)
    }
    fn deallocate(&mut self, block: MemoryBlock, _align: usize) {
        if block.is_empty() { return; }
        unsafe { let _ = VirtualFree(block.ptr as *mut c_void, 0, MEM_RELEASE); }
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
        let cap = Alignment::align_up(capacity, allocation_granularity());
        let base = unsafe { vm_reserve(cap) };
        Self { base, capacity: cap, committed: 0, offset: 0 }
    }
    pub fn offset(&self) -> usize { self.offset }
    pub fn reset_to(&mut self, off: usize) { self.offset = off.min(self.capacity) }
}

impl Drop for FrameAllocatorResource {
    fn drop(&mut self) { unsafe { if !self.base.is_null() { let _ = vm_release(self.base); } } }
}

impl IMemoryResource for FrameAllocatorResource {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        if !Alignment::is_power_of_two(align) { return Err(MemoryError::InvalidArgument); }
        let start = Alignment::align_up(self.offset, align);
        let end = start.checked_add(size).ok_or(MemoryError::Failed)?;
        if end > self.capacity { return Err(MemoryError::OutOfMemory); }
        if end > self.committed {
            let ps = page_size();
            let commit_end = Alignment::align_up(end, ps);
            let commit_from = self.committed;
            let commit_size = commit_end.saturating_sub(commit_from);
            if commit_size > 0 {
                let ok = unsafe { vm_commit(unsafe { self.base.add(commit_from) }, commit_size) };
                if !ok { return Err(MemoryError::OutOfMemory); }
                self.committed = commit_end;
            }
        }
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
    fn reset(&mut self) {
        if self.committed > 0 {
            let _ = unsafe { vm_decommit(self.base, self.committed) };
            self.committed = 0;
        }
        self.offset = 0;
    }
}

impl StackAllocatorResource {
    pub fn new(capacity: usize) -> Self {
        let cap = Alignment::align_up(capacity, allocation_granularity());
        let base = unsafe { vm_reserve(cap) };
        Self { base, capacity: cap, committed: 0, offset: 0 }
    }
    pub fn marker(&self) -> usize { self.offset }
    pub fn reset_to(&mut self, off: usize) { self.offset = off.min(self.capacity) }
}

impl Drop for StackAllocatorResource {
    fn drop(&mut self) { unsafe { if !self.base.is_null() { let _ = vm_release(self.base); } } }
}

impl IMemoryResource for StackAllocatorResource {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        if !Alignment::is_power_of_two(align) { return Err(MemoryError::InvalidArgument); }
        let start = Alignment::align_up(self.offset, align);
        let end = start.checked_add(size).ok_or(MemoryError::Failed)?;
        if end > self.capacity { return Err(MemoryError::OutOfMemory); }
        if end > self.committed {
            let ps = page_size();
            let commit_end = Alignment::align_up(end, ps);
            let commit_from = self.committed;
            let commit_size = commit_end.saturating_sub(commit_from);
            if commit_size > 0 {
                let ok = unsafe { vm_commit(unsafe { self.base.add(commit_from) }, commit_size) };
                if !ok { return Err(MemoryError::OutOfMemory); }
                self.committed = commit_end;
            }
        }
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
            let ps = page_size();
            let decommit_from = Alignment::align_up(self.offset, ps);
            if self.committed > decommit_from {
                let size = self.committed - decommit_from;
                let _ = unsafe { vm_decommit(unsafe { self.base.add(decommit_from) }, size) };
                self.committed = decommit_from;
            }
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
    fn reset(&mut self) {
        if self.committed > 0 {
            let _ = unsafe { vm_decommit(self.base, self.committed) };
            self.committed = 0;
        }
        self.offset = 0;
    }
}

impl PoolAllocatorResource {
    pub fn new(block_size: usize, capacity: usize, alignment: usize) -> Self {
        let bs = core::cmp::max(block_size, 1);
        let al = if Alignment::is_power_of_two(alignment) { alignment } else { Alignment::DEFAULT };
        let cap = Alignment::align_up(capacity, allocation_granularity());
        let base = unsafe { vm_reserve(cap) };
        Self { base, capacity: cap, committed: AtomicUsize::new(0), block_size: bs, alignment: al, owner_thread_id: unsafe { GetCurrentThreadId() as usize }, remote_free_head: AtomicPtr::new(core::ptr::null_mut()) }
    }
}

impl Drop for PoolAllocatorResource {
    fn drop(&mut self) { unsafe { if !self.base.is_null() { let _ = vm_release(self.base); } } }
}

thread_local! {
    static POOL_TLS: core::cell::UnsafeCell<PoolTls> = core::cell::UnsafeCell::new(PoolTls::new());
}

#[derive(Clone, Copy)]
struct PoolTlsEntry { key: usize, head: *mut u8 }
struct PoolTls { entries: [PoolTlsEntry; 64], count: usize }
impl PoolTls {
    fn new() -> Self { Self { entries: core::array::from_fn(|_| PoolTlsEntry{ key: 0, head: core::ptr::null_mut() }), count: 0 } }
    fn get_head_ptr(&mut self, key: usize) -> *mut *mut u8 {
        for i in 0..self.count { if self.entries[i].key == key { return &mut self.entries[i].head; } }
        if self.count < self.entries.len() {
            let idx = self.count; self.entries[idx].key = key; self.entries[idx].head = core::ptr::null_mut(); self.count += 1; return &mut self.entries[idx].head;
        }
        &mut self.entries[0].head
    }
}

fn tls_pop(key: usize) -> *mut u8 {
    let mut out: *mut u8 = core::ptr::null_mut();
    POOL_TLS.with(|c| unsafe {
        let tls = &mut *c.get();
        let hp = tls.get_head_ptr(key);
        let h = *hp;
        if !h.is_null() {
            let next = *(h as *mut *mut u8);
            *hp = next;
            out = h;
        }
    });
    out
}

fn tls_push(key: usize, p: *mut u8) {
    POOL_TLS.with(|c| unsafe {
        let tls = &mut *c.get();
        let hp = tls.get_head_ptr(key);
        let h = *hp;
        *(p as *mut *mut u8) = h;
        *hp = p;
    });
}

impl IMemoryResource for PoolAllocatorResource {
    fn allocate(&mut self, size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if size == 0 { return Err(MemoryError::InvalidArgument); }
        if !Alignment::is_power_of_two(align) { return Err(MemoryError::InvalidArgument); }
        if size > self.block_size { return Err(MemoryError::NotSupported); }
        // 1) Thread-local cache
        let key = self.base as usize;
        unsafe {
            let p_tls = tls_pop(key);
            if !p_tls.is_null() { return Ok(MemoryBlock::new(p_tls, self.block_size)); }
        }
        // 2) Remote free queue pop (lock-free)
        loop {
            let head = self.remote_free_head.load(Ordering::Acquire);
            if head.is_null() { break; }
            unsafe {
                let next = *(head as *mut *mut u8);
                if self.remote_free_head.compare_exchange(head, next, Ordering::AcqRel, Ordering::Acquire).is_ok() {
                    return Ok(MemoryBlock::new(head, self.block_size));
                }
            }
        }
        // 3) Commit new block from reserved space
        let ps = page_size();
        let cur = self.committed.load(Ordering::Acquire);
        let start = Alignment::align_up(cur, self.block_size.max(align));
        let end = start.checked_add(self.block_size).ok_or(MemoryError::Failed)?;
        if end > self.capacity { return Err(MemoryError::OutOfMemory); }
        if end > cur {
            let commit_end = Alignment::align_up(end, ps);
            let commit_size = commit_end.saturating_sub(cur);
            if commit_size > 0 {
                let ok = unsafe { vm_commit(self.base.add(cur), commit_size) };
                if !ok { return Err(MemoryError::OutOfMemory); }
                self.committed.store(commit_end, Ordering::Release);
            }
        }
        unsafe { Ok(MemoryBlock::new(self.base.add(start), self.block_size)) }
    }
    fn deallocate(&mut self, block: MemoryBlock, _align: usize) {
        if block.is_empty() { return; }
        let cur_tid = unsafe { GetCurrentThreadId() as usize };
        unsafe {
            if cur_tid == self.owner_thread_id {
                tls_push(self.base as usize, block.ptr);
            } else {
                // push to remote free list (lock-free)
                loop {
                    let head = self.remote_free_head.load(Ordering::Acquire);
                    *(block.ptr as *mut *mut u8) = head;
                    if self.remote_free_head.compare_exchange(head, block.ptr, Ordering::AcqRel, Ordering::Acquire).is_ok() { break; }
                }
            }
        }
    }
    fn reallocate(&mut self, block: MemoryBlock, new_size: usize, align: usize) -> Result<MemoryBlock, MemoryError> {
        if new_size == 0 { if !block.is_empty() { self.deallocate(block, align); } return Ok(MemoryBlock::empty()); }
        if new_size > self.block_size { return Err(MemoryError::NotSupported); }
        Ok(block)
    }
    fn is_equal(&self, other: &dyn IMemoryResource) -> bool { core::ptr::eq(self as &dyn IMemoryResource, other) }
    fn reset(&mut self) {
        let committed = self.committed.swap(0, Ordering::AcqRel);
        if committed > 0 { let _ = unsafe { vm_decommit(self.base, committed) }; }
        self.remote_free_head.store(core::ptr::null_mut(), Ordering::Release);
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

impl Drop for SmallObjectAllocatorResource {
    fn drop(&mut self) {
        // nothing: inner pools will be dropped with struct
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
        // if new size fits same class, keep block
        for bs in SIZE_CLASSES.iter() { if block.size == *bs && new_size <= *bs && align <= *bs { return Ok(block); } }
        // otherwise allocate new and copy
        let nb = self.allocate(new_size, align)?;
        unsafe { core::ptr::copy_nonoverlapping(block.ptr, nb.ptr, core::cmp::min(block.size, nb.size)); }
        self.deallocate(block, align);
        Ok(nb)
    }
    fn is_equal(&self, other: &dyn IMemoryResource) -> bool { core::ptr::eq(self as &dyn IMemoryResource, other) }
    fn reset(&mut self) {
        for p in self.pools.iter_mut() { p.reset(); }
    }
}

impl<'a> TaggedAllocatorResource<'a> {
    pub fn new(inner: &'a mut dyn IMemoryResource, tag_id: usize, limit: usize) -> Self {
        Self { inner: inner as *mut (dyn IMemoryResource + 'a), tag_id, limit, used: AtomicUsize::new(0) }
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
