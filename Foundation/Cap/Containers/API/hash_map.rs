use cap_memory::{Allocator, MemoryBlock, MemoryError};
use core::hash::{Hash, Hasher};
use core::mem::MaybeUninit;

struct FnvHasher(u64);
impl FnvHasher { fn new() -> Self { Self(0xcbf29ce484222325) } }
impl Hasher for FnvHasher {
    fn write(&mut self, bytes: &[u8]) { for &b in bytes { self.0 ^= b as u64; self.0 = self.0.wrapping_mul(1099511628211); } }
    fn finish(&self) -> u64 { self.0 }
}

struct Slot<K, V> { key: MaybeUninit<K>, val: MaybeUninit<V>, used: u8 }

pub struct HashMap<'a, K: Copy + Eq + Hash, V: Copy> { ptr: *mut Slot<K, V>, cap: usize, mask: usize, len: usize, blk: MemoryBlock, alloc: Allocator<'a> }

impl<'a, K: Copy + Eq + Hash, V: Copy> HashMap<'a, K, V> {
    pub fn with_capacity(alloc: Allocator<'a>, capacity_pow2: usize) -> Result<Self, MemoryError> {
        if capacity_pow2 == 0 || (capacity_pow2 & (capacity_pow2 - 1)) != 0 { return Err(MemoryError::InvalidArgument); }
        let bytes = capacity_pow2.checked_mul(core::mem::size_of::<Slot<K, V>>()).ok_or(MemoryError::Failed)?;
        let blk = alloc.alloc(bytes, core::mem::align_of::<Slot<K, V>>())?;
        let ptr = blk.ptr.cast::<Slot<K, V>>();
        for i in 0..capacity_pow2 { unsafe { core::ptr::write(ptr.add(i), Slot { key: MaybeUninit::uninit(), val: MaybeUninit::uninit(), used: 0 }) } }
        Ok(Self { ptr, cap: capacity_pow2, mask: capacity_pow2 - 1, len: 0, blk, alloc })
    }
    fn hash_key(k: &K) -> u64 { let mut h = FnvHasher::new(); k.hash(&mut h); h.finish() }
    pub fn insert(&mut self, k: K, v: V) -> Result<(), MemoryError> {
        let mut i = (Self::hash_key(&k) as usize) & self.mask; let mut probes = 0;
        loop {
            unsafe {
                let s = &mut *self.ptr.add(i);
                if s.used == 0 {
                    s.key.write(k);
                    s.val.write(v);
                    s.used = 1;
                    self.len += 1;
                    return Ok(());
                } else {
                    let sk = s.key.assume_init_ref();
                    if *sk == k {
                        s.val.write(v);
                        return Ok(());
                    }
                }
            }
            i = (i + 1) & self.mask; probes += 1; if probes > self.cap { return Err(MemoryError::OutOfMemory); }
        }
    }
    pub fn get(&self, k: K) -> Option<V> {
        let mut i = (Self::hash_key(&k) as usize) & self.mask; let mut probes = 0;
        loop {
            unsafe {
                let s = &*self.ptr.add(i);
                if s.used == 0 { return None; }
                let sk = s.key.assume_init_ref();
                if *sk == k { return Some(s.val.assume_init()); }
            }
            i = (i + 1) & self.mask; probes += 1; if probes > self.cap { return None; }
        }
    }
    pub fn remove(&mut self, k: K) -> bool {
        let mut i = (Self::hash_key(&k) as usize) & self.mask; let mut probes = 0;
        loop {
            unsafe {
                let s = &mut *self.ptr.add(i);
                if s.used == 0 { return false; }
                let sk = s.key.assume_init_ref();
                if *sk == k { s.used = 0; self.len -= 1; return true; }
            }
            i = (i + 1) & self.mask; probes += 1; if probes > self.cap { return false; }
        }
    }
    pub fn len(&self) -> usize { self.len }
}

impl<'a, K: Copy + Eq + Hash, V: Copy> Drop for HashMap<'a, K, V> { fn drop(&mut self) { if !self.blk.is_empty() { self.alloc.free(self.blk, core::mem::align_of::<Slot<K, V>>()); } } }

pub type HashMapU64U64<'a> = HashMap<'a, u64, u64>;
