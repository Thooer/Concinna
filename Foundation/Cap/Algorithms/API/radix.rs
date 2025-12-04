use cap_memory::{Allocator, MemoryBlock, MemoryError};

pub fn radix_sort_u32(mut alloc: Allocator, xs: &mut [u32]) -> Result<usize, MemoryError> {
    if xs.len() <= 1 { return Ok(xs.len()); }
    let n = xs.len();
    let bytes = n.checked_mul(core::mem::size_of::<u32>()).ok_or(MemoryError::Failed)?;
    let tmp = alloc.alloc(bytes, core::mem::align_of::<u32>())?;
    let out = tmp.ptr.cast::<u32>();
    let mut count = [0u32; 256];
    let mut pref = [0u32; 256];
    for pass in 0..4 {
        for i in 0..256 { count[i] = 0; }
        for &v in xs.iter() { let b = ((v >> (pass*8)) & 0xFF) as usize; count[b] += 1; }
        let mut acc = 0u32; for i in 0..256 { pref[i] = acc; acc += count[i]; }
        for &v in xs.iter() { let b = ((v >> (pass*8)) & 0xFF) as usize; let p = pref[b] as usize; unsafe { out.add(p).write(v); } pref[b] += 1; }
        unsafe { core::ptr::copy_nonoverlapping(out, xs.as_mut_ptr(), n); }
    }
    let _ = alloc.free(tmp, core::mem::align_of::<u32>());
    Ok(n)
}

pub fn radix_sort_u64(mut alloc: Allocator, xs: &mut [u64]) -> Result<usize, MemoryError> {
    if xs.len() <= 1 { return Ok(xs.len()); }
    let n = xs.len();
    let bytes = n.checked_mul(core::mem::size_of::<u64>()).ok_or(MemoryError::Failed)?;
    let tmp = alloc.alloc(bytes, core::mem::align_of::<u64>())?;
    let out = tmp.ptr.cast::<u64>();
    let mut count = [0u32; 256];
    let mut pref = [0u32; 256];
    for pass in 0..8 {
        for i in 0..256 { count[i] = 0; }
        for &v in xs.iter() { let b = ((v >> (pass*8)) & 0xFF) as usize; count[b] += 1; }
        let mut acc = 0u32; for i in 0..256 { pref[i] = acc; acc += count[i]; }
        for &v in xs.iter() { let b = ((v >> (pass*8)) & 0xFF) as usize; let p = pref[b] as usize; unsafe { out.add(p).write(v); } pref[b] += 1; }
        unsafe { core::ptr::copy_nonoverlapping(out, xs.as_mut_ptr(), n); }
    }
    let _ = alloc.free(tmp, core::mem::align_of::<u64>());
    Ok(n)
}

