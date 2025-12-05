use cap_memory::{Allocator, MemoryBlock, MemoryError};
use core::hash::{Hash, Hasher};
use core::mem::MaybeUninit;
use core::ptr::{read, write, drop_in_place};

struct FnvHasher(u64);
impl FnvHasher { fn new() -> Self { Self(0xcbf29ce484222325) } }
impl Hasher for FnvHasher {
    fn write(&mut self, bytes: &[u8]) { for &b in bytes { self.0 ^= b as u64; self.0 = self.0.wrapping_mul(1099511628211); } }
    fn finish(&self) -> u64 { self.0 }
}

const SLOT_EMPTY: u8 = 0;
const SLOT_USED: u8 = 1;
const SLOT_DELETED: u8 = 2;

struct Slot<K, V> { key: MaybeUninit<K>, val: MaybeUninit<V>, used: u8 }

pub struct HashMap<'a, K: Eq + Hash, V> { ptr: *mut Slot<K, V>, cap: usize, mask: usize, len: usize, blk: MemoryBlock, alloc: Allocator<'a> }

unsafe impl<'a, K: Eq + Hash + Send, V: Send> Send for HashMap<'a, K, V> {}
unsafe impl<'a, K: Eq + Hash + Sync, V: Sync> Sync for HashMap<'a, K, V> {}

impl<'a, K: Eq + Hash, V> HashMap<'a, K, V> {
    pub fn with_capacity(alloc: Allocator<'a>, capacity_pow2: usize) -> Result<Self, MemoryError> {
        if capacity_pow2 == 0 || (capacity_pow2 & (capacity_pow2 - 1)) != 0 { return Err(MemoryError::InvalidArgument); }
        let bytes = capacity_pow2.checked_mul(core::mem::size_of::<Slot<K, V>>()).ok_or(MemoryError::Failed)?;
        let blk = alloc.alloc(bytes, core::mem::align_of::<Slot<K, V>>())?;
        let ptr = blk.ptr.cast::<Slot<K, V>>();
        for i in 0..capacity_pow2 { unsafe { core::ptr::write(ptr.add(i), Slot { key: MaybeUninit::uninit(), val: MaybeUninit::uninit(), used: SLOT_EMPTY }) } }
        Ok(Self { ptr, cap: capacity_pow2, mask: capacity_pow2 - 1, len: 0, blk, alloc })
    }
    
    pub fn capacity(&self) -> usize { self.cap }

    fn hash_key(k: &K) -> u64 { let mut h = FnvHasher::new(); k.hash(&mut h); h.finish() }
    
    pub fn insert(&mut self, k: K, v: V) -> Result<(), MemoryError> {
        if self.len >= self.cap / 2 { return Err(MemoryError::OutOfMemory); } // Simple load factor check, TODO: Resize
        
        let mut i = (Self::hash_key(&k) as usize) & self.mask; let mut probes = 0;
        let mut first_deleted = None;
        
        loop {
            unsafe {
                let s = &mut *self.ptr.add(i);
                if s.used == SLOT_EMPTY {
                    // Found empty slot
                    let target = if let Some(d_idx) = first_deleted { &mut *self.ptr.add(d_idx) } else { s };
                    target.key.write(k);
                    target.val.write(v);
                    target.used = SLOT_USED;
                    self.len += 1;
                    return Ok(());
                } else if s.used == SLOT_DELETED {
                    if first_deleted.is_none() { first_deleted = Some(i); }
                } else if s.used == SLOT_USED {
                    let sk = s.key.assume_init_ref();
                    if *sk == k {
                        // Replace value
                        drop_in_place(s.val.as_mut_ptr()); // Drop old value
                        s.val.write(v);
                        return Ok(());
                    }
                }
            }
            i = (i + 1) & self.mask; probes += 1; if probes > self.cap { return Err(MemoryError::OutOfMemory); }
        }
    }
    
    pub fn get(&self, k: &K) -> Option<&V> {
        let mut i = (Self::hash_key(k) as usize) & self.mask; let mut probes = 0;
        loop {
            unsafe {
                let s = &*self.ptr.add(i);
                if s.used == SLOT_EMPTY { return None; }
                if s.used == SLOT_USED {
                    let sk = s.key.assume_init_ref();
                    if *sk == *k { return Some(s.val.assume_init_ref()); }
                }
            }
            i = (i + 1) & self.mask; probes += 1; if probes > self.cap { return None; }
        }
    }

    pub fn get_mut(&mut self, k: &K) -> Option<&mut V> {
        let mut i = (Self::hash_key(k) as usize) & self.mask; let mut probes = 0;
        loop {
            unsafe {
                let s = &mut *self.ptr.add(i);
                if s.used == SLOT_EMPTY { return None; }
                if s.used == SLOT_USED {
                    let sk = s.key.assume_init_ref();
                    if *sk == *k { return Some(s.val.assume_init_mut()); }
                }
            }
            i = (i + 1) & self.mask; probes += 1; if probes > self.cap { return None; }
        }
    }
    
    pub fn remove(&mut self, k: &K) -> Option<V> {
        let mut i = (Self::hash_key(k) as usize) & self.mask; let mut probes = 0;
        loop {
            unsafe {
                let s = &mut *self.ptr.add(i);
                if s.used == SLOT_EMPTY { return None; }
                if s.used == SLOT_USED {
                    let sk = s.key.assume_init_ref();
                    if *sk == *k {
                         let v = read(s.val.as_ptr());
                         drop_in_place(s.key.as_mut_ptr()); // Drop key
                         s.used = SLOT_DELETED;
                         self.len -= 1;
                         return Some(v);
                    }
                }
            }
            i = (i + 1) & self.mask; probes += 1; if probes > self.cap { return None; }
        }
    }
    
    pub fn len(&self) -> usize { self.len }
    pub fn is_empty(&self) -> bool { self.len == 0 }

    // Simple iterator
    pub fn iter(&self) -> Iter<'_, K, V> {
        Iter { ptr: self.ptr, end: unsafe { self.ptr.add(self.cap) }, _marker: core::marker::PhantomData }
    }

    pub fn iter_mut(&mut self) -> IterMut<'_, K, V> {
        IterMut { ptr: self.ptr, end: unsafe { self.ptr.add(self.cap) }, _marker: core::marker::PhantomData }
    }
    
    pub fn values_mut(&mut self) -> ValuesMut<'_, K, V> {
        ValuesMut { iter: self.iter_mut() }
    }
}

pub struct Iter<'a, K, V> { ptr: *mut Slot<K, V>, end: *mut Slot<K, V>, _marker: core::marker::PhantomData<&'a Slot<K, V>> }

unsafe impl<'a, K: Sync, V: Sync> Send for Iter<'a, K, V> {}
unsafe impl<'a, K: Sync, V: Sync> Sync for Iter<'a, K, V> {}

impl<'a, K, V> Iterator for Iter<'a, K, V> {
    type Item = (&'a K, &'a V);
    fn next(&mut self) -> Option<Self::Item> {
        while self.ptr < self.end {
            unsafe {
                let s = &*self.ptr;
                self.ptr = self.ptr.add(1);
                if s.used == SLOT_USED {
                    return Some((s.key.assume_init_ref(), s.val.assume_init_ref()));
                }
            }
        }
        None
    }
}

pub struct IterMut<'a, K, V> { ptr: *mut Slot<K, V>, end: *mut Slot<K, V>, _marker: core::marker::PhantomData<&'a mut Slot<K, V>> }

unsafe impl<'a, K: Sync, V: Send> Send for IterMut<'a, K, V> {}
unsafe impl<'a, K: Sync, V: Sync> Sync for IterMut<'a, K, V> {}

impl<'a, K, V> Iterator for IterMut<'a, K, V> {
    type Item = (&'a K, &'a mut V);
    fn next(&mut self) -> Option<Self::Item> {
        while self.ptr < self.end {
            unsafe {
                let s = &mut *self.ptr;
                self.ptr = self.ptr.add(1);
                if s.used == SLOT_USED {
                    return Some((s.key.assume_init_ref(), s.val.assume_init_mut()));
                }
            }
        }
        None
    }
}

pub struct ValuesMut<'a, K, V> { iter: IterMut<'a, K, V> }

unsafe impl<'a, K: Sync, V: Send> Send for ValuesMut<'a, K, V> {}
unsafe impl<'a, K: Sync, V: Sync> Sync for ValuesMut<'a, K, V> {}

impl<'a, K, V> Iterator for ValuesMut<'a, K, V> {
    type Item = &'a mut V;
    fn next(&mut self) -> Option<Self::Item> { self.iter.next().map(|(_, v)| v) }
}


impl<'a, K: Eq + Hash, V> Drop for HashMap<'a, K, V> {
    fn drop(&mut self) {
        if !self.blk.is_empty() {
             unsafe {
                for i in 0..self.cap {
                    let s = &mut *self.ptr.add(i);
                    if s.used == SLOT_USED {
                        drop_in_place(s.key.as_mut_ptr());
                        drop_in_place(s.val.as_mut_ptr());
                    }
                }
            }
            self.alloc.free(self.blk, core::mem::align_of::<Slot<K, V>>());
        }
    }
}

pub type HashMapU64U64<'a> = HashMap<'a, u64, u64>;
