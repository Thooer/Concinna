use cap_memory::{Allocator, MemoryBlock, MemoryError};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum VfsError { Failed, NotMounted, BufferTooSmall }

struct Entry { alias: [u8; 16], alias_len: usize, root_blk: MemoryBlock, root_len: usize }

pub struct Vfs<'a> { entries: *mut Entry, cap: usize, len: usize, blk: MemoryBlock, alloc: Allocator<'a> }

impl<'a> Vfs<'a> {
    pub fn with_capacity(mut alloc: Allocator<'a>, capacity: usize) -> Result<Self, MemoryError> {
        let bytes = capacity.checked_mul(core::mem::size_of::<Entry>()).ok_or(MemoryError::Failed)?;
        let blk = alloc.alloc(bytes, core::mem::align_of::<Entry>())?;
        Ok(Self { entries: blk.ptr.cast::<Entry>(), cap: capacity, len: 0, blk, alloc })
    }
    pub fn mount(&mut self, alias: &str, root: &str) -> Result<(), VfsError> {
        if self.len >= self.cap { return Err(VfsError::Failed); }
        if alias.len() == 0 || alias.len() > 16 { return Err(VfsError::Failed); }
        let rbytes = root.len();
        let rblk = self.alloc.alloc(rbytes, 1).map_err(|_| VfsError::Failed)?;
        unsafe { core::ptr::copy_nonoverlapping(root.as_ptr(), rblk.ptr, rbytes); }
        let idx = self.len;
        let mut e = Entry { alias: [0u8; 16], alias_len: alias.len(), root_blk: rblk, root_len: rbytes };
        unsafe {
            core::ptr::copy_nonoverlapping(alias.as_ptr(), e.alias.as_mut_ptr(), alias.len());
            self.entries.add(idx).write(e);
        }
        self.len += 1;
        Ok(())
    }
    fn find_alias(&self, alias: &str) -> Option<usize> {
        for i in 0..self.len {
            unsafe {
                let e = self.entries.add(i).read();
                if e.alias_len == alias.len() {
                    let ea = &e.alias[..e.alias_len];
                    if ea == alias.as_bytes() { return Some(i); }
                }
            }
        }
        None
    }
    pub fn resolve(&self, vpath: &str, out: &mut [u8]) -> Result<usize, VfsError> {
        let b = vpath.as_bytes();
        let mut pos = 0usize;
        while pos < b.len() && b[pos] != b':' { pos += 1; }
        if pos == 0 || pos >= b.len() { return Err(VfsError::Failed); }
        let alias = unsafe { core::str::from_utf8_unchecked(&b[..pos]) };
        let rel = if pos+1 < b.len() { unsafe { core::str::from_utf8_unchecked(&b[(pos+1)..]) } } else { "" };
        let idx = self.find_alias(alias).ok_or(VfsError::NotMounted)?;
        unsafe {
            let e = self.entries.add(idx).read();
            let root = core::slice::from_raw_parts(e.root_blk.ptr, e.root_len);
            let root_str = core::str::from_utf8_unchecked(root);
            cap_path::join(root_str, rel, out).map_err(|_| VfsError::BufferTooSmall)
        }
    }
    pub fn open_read(&self, vpath: &str) -> Result<prm_file::FileHandle, VfsError> {
        let mut buf = [0u8; 1024];
        let n = self.resolve(vpath, &mut buf).map_err(|_| VfsError::Failed)?;
        let path = unsafe { core::str::from_utf8_unchecked(&buf[..n]) };
        prm_file::open(path, prm_file::FileOpenMode::Read, prm_file::FileShareMode::Read).map_err(|_| VfsError::Failed)
    }
}

impl<'a> Drop for Vfs<'a> {
    fn drop(&mut self) {
        unsafe {
            for i in 0..self.len {
                let e = self.entries.add(i).read();
                if !e.root_blk.is_empty() { let _ = self.alloc.free(e.root_blk, 1); }
            }
            if !self.blk.is_empty() { let _ = self.alloc.free(self.blk, core::mem::align_of::<Entry>()); }
        }
    }
}

