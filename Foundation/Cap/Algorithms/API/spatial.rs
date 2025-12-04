use cap_memory::{Allocator, MemoryBlock, MemoryError};
use cap_math::{AABB, Vec3};

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct LBVHNode { pub left: i32, pub right: i32, pub parent: i32, pub bbox: AABB }

fn union(a: AABB, b: AABB) -> AABB {
    let min = Vec3 { x: a.min.x.min(b.min.x), y: a.min.y.min(b.min.y), z: a.min.z.min(b.min.z) };
    let max = Vec3 { x: a.max.x.max(b.max.x), y: a.max.y.max(b.max.y), z: a.max.z.max(b.max.z) };
    AABB { min, max }
}

fn bbox_of_points(ps: &[Vec3]) -> AABB {
    if ps.is_empty() { return AABB { min: Vec3 { x: 0.0, y: 0.0, z: 0.0 }, max: Vec3 { x: 0.0, y: 0.0, z: 0.0 } }; }
    let mut min = ps[0];
    let mut max = ps[0];
    for &p in ps.iter().skip(1) {
        if p.x < min.x { min.x = p.x; } if p.y < min.y { min.y = p.y; } if p.z < min.z { min.z = p.z; }
        if p.x > max.x { max.x = p.x; } if p.y > max.y { max.y = p.y; } if p.z > max.z { max.z = p.z; }
    }
    AABB { min, max }
}

fn morton3(p: Vec3, bb: AABB) -> u32 {
    let ex = (bb.max.x - bb.min.x).abs();
    let ey = (bb.max.y - bb.min.y).abs();
    let ez = (bb.max.z - bb.min.z).abs();
    let nx = if ex > 0.0 { (p.x - bb.min.x) / ex } else { 0.0 };
    let ny = if ey > 0.0 { (p.y - bb.min.y) / ey } else { 0.0 };
    let nz = if ez > 0.0 { (p.z - bb.min.z) / ez } else { 0.0 };
    let fx = if nx < 0.0 { 0.0 } else if nx > 1.0 { 1.0 } else { nx };
    let fy = if ny < 0.0 { 0.0 } else if ny > 1.0 { 1.0 } else { ny };
    let fz = if nz < 0.0 { 0.0 } else if nz > 1.0 { 1.0 } else { nz };
    let ix = (fx * 1023.0) as u32;
    let iy = (fy * 1023.0) as u32;
    let iz = (fz * 1023.0) as u32;
    let mut code = 0u32;
    for i in 0..10 {
        let bx = (ix >> i) & 1;
        let by = (iy >> i) & 1;
        let bz = (iz >> i) & 1;
        code |= bx << (3 * i);
        code |= by << (3 * i + 1);
        code |= bz << (3 * i + 2);
    }
    code
}

fn radix_sort_pairs_u32(mut alloc: Allocator, codes: &mut [u32], indices: &mut [u32]) -> Result<(), MemoryError> {
    let n = codes.len();
    if indices.len() < n { return Err(MemoryError::Failed); }
    if n <= 1 { return Ok(()); }
    let bytes = n.checked_mul(core::mem::size_of::<u32>()).ok_or(MemoryError::Failed)?;
    let tmp = alloc.alloc(bytes * 2, core::mem::align_of::<u32>())?;
    let out_codes = tmp.ptr.cast::<u32>();
    let out_indices = unsafe { out_codes.add(n) };
    let mut count = [0u32; 256];
    let mut pref = [0u32; 256];
    for pass in 0..4 {
        for i in 0..256 { count[i] = 0; }
        for &v in codes.iter() { let b = ((v >> (pass * 8)) & 0xFF) as usize; count[b] += 1; }
        let mut acc = 0u32; for i in 0..256 { pref[i] = acc; acc += count[i]; }
        for i in 0..n {
            let v = codes[i]; let b = ((v >> (pass * 8)) & 0xFF) as usize; let p = pref[b] as usize;
            unsafe { out_codes.add(p).write(v); out_indices.add(p).write(indices[i]); }
            pref[b] += 1;
        }
        unsafe { core::ptr::copy_nonoverlapping(out_codes, codes.as_mut_ptr(), n); core::ptr::copy_nonoverlapping(out_indices, indices.as_mut_ptr(), n); }
    }
    let _ = alloc.free(tmp, core::mem::align_of::<u32>());
    Ok(())
}

pub fn build_lbvh(mut alloc: Allocator, prims: &[AABB], centers: &[Vec3], out_nodes: &mut [LBVHNode]) -> Result<usize, MemoryError> {
    let n = core::cmp::min(prims.len(), centers.len());
    if n == 0 { return Ok(0); }
    let needed = n.checked_mul(2).and_then(|v| v.checked_sub(1)).ok_or(MemoryError::Failed)?;
    if out_nodes.len() < needed { return Err(MemoryError::OutOfMemory); }
    let world = bbox_of_points(&centers[..n]);
    // scratch arrays
    let bytes = n.checked_mul(core::mem::size_of::<u32>()).ok_or(MemoryError::Failed)?;
    let blk = alloc.alloc(bytes * 2, core::mem::align_of::<u32>())?;
    let codes = blk.ptr.cast::<u32>();
    let indices = unsafe { codes.add(n) };
    for i in 0..n {
        let c = morton3(centers[i], world);
        unsafe { codes.add(i).write(c); indices.add(i).write(i as u32); }
    }
    // sort by morton code while carrying indices
    {
        let mut codes_slice = unsafe { core::slice::from_raw_parts_mut(codes, n) };
        let mut indices_slice = unsafe { core::slice::from_raw_parts_mut(indices, n) };
        radix_sort_pairs_u32(alloc, &mut codes_slice, &mut indices_slice)?;
    }
    // build leaves
    for i in 0..n {
        let prim_idx = unsafe { *indices.add(i) } as usize;
        out_nodes[i] = LBVHNode { left: -1, right: -1, parent: -1, bbox: prims[prim_idx] };
    }
    // build internal nodes bottom-up
    // level indices buffer
    let level_bytes = n.checked_mul(core::mem::size_of::<u32>()).ok_or(MemoryError::Failed)?;
    let level_blk = alloc.alloc(level_bytes, core::mem::align_of::<u32>())?;
    let mut cur_level = unsafe { core::slice::from_raw_parts_mut(level_blk.ptr.cast::<u32>(), n) };
    for i in 0..n { cur_level[i] = i as u32; }
    let mut cur_count = n;
    let mut next_free = n;
    while cur_count > 1 {
        let pairs = cur_count / 2; let has_odd = (cur_count & 1) != 0;
        let mut next_level_count = pairs + (has_odd as usize);
        for k in 0..pairs {
            let li = cur_level[2 * k] as usize;
            let ri = cur_level[2 * k + 1] as usize;
            let parent = next_free as i32;
            out_nodes[li].parent = parent;
            out_nodes[ri].parent = parent;
            let bb = union(out_nodes[li].bbox, out_nodes[ri].bbox);
            out_nodes[next_free] = LBVHNode { left: li as i32, right: ri as i32, parent: -1, bbox: bb };
            cur_level[k] = next_free as u32;
            next_free += 1;
        }
        if has_odd {
            // propagate the last node upward
            let li = cur_level[cur_count - 1] as usize;
            out_nodes[li].parent = next_free as i32;
            out_nodes[next_free] = LBVHNode { left: li as i32, right: -1, parent: -1, bbox: out_nodes[li].bbox };
            cur_level[pairs] = next_free as u32;
            next_free += 1;
        }
        cur_count = next_level_count;
    }
    let _ = alloc.free(level_blk, core::mem::align_of::<u32>());
    let _ = alloc.free(blk, core::mem::align_of::<u32>());
    Ok(next_free)
}

