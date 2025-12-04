use cap_memory::{Allocator, MemoryBlock, MemoryError};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ResourceError { Failed, BufferTooSmall }

pub struct VersionedBuffer<'a> { slots: [MemoryBlock; 3], size: usize, align: usize, read_idx: usize, write_idx: usize, alloc: Allocator<'a> }

impl<'a> VersionedBuffer<'a> {
    pub fn new(mut alloc: Allocator<'a>, size: usize, align: usize) -> Result<Self, MemoryError> {
        let s0 = alloc.alloc(size, align)?;
        let s1 = alloc.alloc(size, align)?;
        let s2 = alloc.alloc(size, align)?;
        Ok(Self { slots: [s0, s1, s2], size, align, read_idx: 0, write_idx: 1, alloc })
    }
    pub fn write_ptr(&self) -> *mut u8 { self.slots[self.write_idx].ptr }
    pub fn read_ptr(&self) -> *mut u8 { self.slots[self.read_idx].ptr }
    pub fn size(&self) -> usize { self.size }
    pub fn commit(&mut self) { self.read_idx = self.write_idx; self.write_idx = (self.write_idx + 1) % 3; }
}

impl<'a> Drop for VersionedBuffer<'a> { fn drop(&mut self) { let _ = self.alloc.free(self.slots[0], self.align); let _ = self.alloc.free(self.slots[1], self.align); let _ = self.alloc.free(self.slots[2], self.align); } }

#[derive(Clone, Copy)]
pub struct ResHandle<'a> { buf: *mut VersionedBuffer<'a> }

pub struct ResourceManager<'a> { entries: *mut ResHandle<'a>, cap: usize, len: usize, blk: MemoryBlock, alloc: Allocator<'a> }

impl<'a> ResourceManager<'a> {
    pub fn with_capacity(mut alloc: Allocator<'a>, capacity: usize) -> Result<Self, MemoryError> {
        let bytes = capacity.checked_mul(core::mem::size_of::<ResHandle>()).ok_or(MemoryError::Failed)?;
        let blk = alloc.alloc(bytes, core::mem::align_of::<ResHandle>())?;
        Ok(Self { entries: blk.ptr.cast::<ResHandle<'a>>(), cap: capacity, len: 0, blk, alloc })
    }
    pub fn create_buffer(&mut self, size: usize, align: usize) -> Result<ResHandle<'a>, MemoryError> {
        if self.len >= self.cap { return Err(MemoryError::Failed); }
        let vb = VersionedBuffer::new(self.alloc, size, align)?;
        let ptr = Box::into_raw(Box::new(vb));
        let h = ResHandle { buf: ptr };
        unsafe { self.entries.add(self.len).write(h); }
        self.len += 1;
        Ok(h)
    }
    pub fn get(&self, idx: usize) -> Option<ResHandle<'a>> { if idx < self.len { Some(unsafe { self.entries.add(idx).read() }) } else { None } }
}

impl<'a> Drop for ResourceManager<'a> {
    fn drop(&mut self) {
        unsafe {
            for i in 0..self.len {
                let h = self.entries.add(i).read();
                if !h.buf.is_null() { let _ = Box::from_raw(h.buf); }
            }
            if !self.blk.is_empty() { let _ = self.alloc.free(self.blk, core::mem::align_of::<ResHandle>()); }
        }
    }
}

impl<'a> ResHandle<'a> {
    pub fn write_ptr(&self) -> *mut u8 { unsafe { (&*self.buf).write_ptr() } }
    pub fn read_ptr(&self) -> *mut u8 { unsafe { (&*self.buf).read_ptr() } }
    pub fn commit(&self) { unsafe { (&mut *self.buf).commit(); } }
}

