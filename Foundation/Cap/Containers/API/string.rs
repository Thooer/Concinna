use cap_memory::{Allocator, MemoryBlock, MemoryError};

const SSO_CAP: usize = 24;

enum Repr<'a> { Inline { buf: [u8; SSO_CAP], len: usize }, Heap { ptr: *mut u8, len: usize, cap: usize, blk: MemoryBlock, _marker: core::marker::PhantomData<&'a ()> } }

pub struct CapString<'a> { repr: Repr<'a>, alloc: Allocator<'a> }

impl<'a> CapString<'a> {
    pub fn with_capacity(alloc: Allocator<'a>, capacity: usize) -> Result<Self, MemoryError> {
        if capacity <= SSO_CAP {
            Ok(Self { repr: Repr::Inline { buf: [0u8; SSO_CAP], len: 0 }, alloc })
        } else {
            let blk = alloc.alloc(capacity, 1)?;
            Ok(Self { repr: Repr::Heap { ptr: blk.ptr, len: 0, cap: capacity, blk, _marker: core::marker::PhantomData }, alloc })
        }
    }
    pub fn from_str(alloc: Allocator<'a>, s: &str) -> Result<Self, MemoryError> {
        let mut cs = Self::with_capacity(alloc, s.len())?; cs.push_str(s)?; Ok(cs)
    }
    pub fn as_bytes(&self) -> &[u8] {
        match &self.repr {
            Repr::Inline { buf, len } => &buf[..*len],
            Repr::Heap { ptr, len, .. } => unsafe { core::slice::from_raw_parts(*ptr, *len) },
        }
    }
    pub fn push_str(&mut self, s: &str) -> Result<(), MemoryError> {
        match &mut self.repr {
            Repr::Inline { buf, len } => {
                let need = (*len).checked_add(s.len()).ok_or(MemoryError::Failed)?;
                if need <= SSO_CAP {
                    unsafe { core::ptr::copy_nonoverlapping(s.as_ptr(), buf.as_mut_ptr().add(*len), s.len()); }
                    *len += s.len();
                    Ok(())
                } else {
                    let nb = self.alloc.alloc(need, 1)?;
                    unsafe {
                        core::ptr::copy_nonoverlapping(buf.as_ptr(), nb.ptr, *len);
                        core::ptr::copy_nonoverlapping(s.as_ptr(), nb.ptr.add(*len), s.len());
                    }
                    self.repr = Repr::Heap { ptr: nb.ptr, len: need, cap: need, blk: nb, _marker: core::marker::PhantomData };
                    Ok(())
                }
            }
            Repr::Heap { ptr, len, cap, blk, .. } => {
                let need = (*len).checked_add(s.len()).ok_or(MemoryError::Failed)?;
                if need > *cap {
                    let nb = self.alloc.realloc(*blk, need, 1)?;
                    *ptr = nb.ptr;
                    *cap = need;
                    *blk = nb;
                }
                unsafe { core::ptr::copy_nonoverlapping(s.as_ptr(), (*ptr).add(*len), s.len()); }
                *len += s.len();
                Ok(())
            }
        }
    }
    pub fn clear(&mut self) {
        match &mut self.repr {
            Repr::Inline { len, .. } => { *len = 0; }
            Repr::Heap { len, .. } => { *len = 0; }
        }
    }
}

impl<'a> Drop for CapString<'a> {
    fn drop(&mut self) {
        if let Repr::Heap { blk, .. } = self.repr { if !blk.is_empty() { self.alloc.free(blk, 1); } }
    }
}
