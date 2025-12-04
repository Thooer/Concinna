use crate::*;

#[cfg(target_arch = "x86_64")]
use std::arch::x86_64::*;

#[inline]
fn scalar_add(a: F32x4, b: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = a.lanes[i] + b.lanes[i]; } F32x4 { lanes: r } }
#[inline]
fn scalar_mul(a: F32x4, b: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = a.lanes[i] * b.lanes[i]; } F32x4 { lanes: r } }
#[inline]
fn scalar_dot(a: F32x4, b: F32x4) -> f32 { let mut s = 0.0; for i in 0..4 { s += a.lanes[i] * b.lanes[i]; } s }
#[inline]
fn scalar_fma(a: F32x4, b: F32x4, c: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = a.lanes[i] * b.lanes[i] + c.lanes[i]; } F32x4 { lanes: r } }
#[inline]
fn scalar_min(a: F32x4, b: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = if a.lanes[i] < b.lanes[i] { a.lanes[i] } else { b.lanes[i] } } F32x4 { lanes: r } }
#[inline]
fn scalar_max(a: F32x4, b: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = if a.lanes[i] > b.lanes[i] { a.lanes[i] } else { b.lanes[i] } } F32x4 { lanes: r } }

#[inline]
fn scalar_abs(a: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = f32::from_bits(a.lanes[i].to_bits() & 0x7FFF_FFFF) } F32x4 { lanes: r } }
#[inline]
fn scalar_rcp(a: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = 1.0 / a.lanes[i] } F32x4 { lanes: r } }
#[inline]
fn scalar_rsqrt(a: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = 1.0 / a.lanes[i].sqrt() } F32x4 { lanes: r } }
#[inline]
fn scalar_floor(a: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = a.lanes[i].floor() } F32x4 { lanes: r } }
#[inline]
fn scalar_ceil(a: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = a.lanes[i].ceil() } F32x4 { lanes: r } }
#[inline]
fn scalar_round(a: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = a.lanes[i].round() } F32x4 { lanes: r } }
#[inline]
fn scalar_trunc(a: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = a.lanes[i].trunc() } F32x4 { lanes: r } }

#[inline(always)]
pub unsafe fn impl_load(ptr: *const f32) -> F32x4 {
    F32x4 { lanes: [*ptr, *ptr.add(1), *ptr.add(2), *ptr.add(3)] }
}

#[inline(always)]
pub unsafe fn impl_store(ptr: *mut f32, v: F32x4) {
    *ptr = v.lanes[0];
    *ptr.add(1) = v.lanes[1];
    *ptr.add(2) = v.lanes[2];
    *ptr.add(3) = v.lanes[3];
}

#[inline(always)]
pub fn impl_add(a: F32x4, b: F32x4) -> F32x4 {
    #[cfg(all(target_arch = "x86_64", target_feature = "sse"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let vb = _mm_loadu_ps(b.lanes.as_ptr());
        let vr = _mm_add_ps(va, vb);
        let mut out = F32x4 { lanes: [0.0;4] };
        _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    scalar_add(a, b)
}

#[inline(always)]
pub fn impl_mul(a: F32x4, b: F32x4) -> F32x4 {
    #[cfg(all(target_arch = "x86_64", target_feature = "sse"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let vb = _mm_loadu_ps(b.lanes.as_ptr());
        let vr = _mm_mul_ps(va, vb);
        let mut out = F32x4 { lanes: [0.0;4] };
        _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    scalar_mul(a, b)
}

#[inline(always)]
pub fn impl_dot(a: F32x4, b: F32x4) -> f32 {
    #[cfg(all(target_arch = "x86_64", target_feature = "sse"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let vb = _mm_loadu_ps(b.lanes.as_ptr());
        let m = _mm_mul_ps(va, vb);
        let shuf = _mm_movehdup_ps(m);
        let sums = _mm_add_ps(m, shuf);
        let shuf2 = _mm_movehl_ps(shuf, sums);
        let sums2 = _mm_add_ss(sums, shuf2);
        return _mm_cvtss_f32(sums2);
    }
    scalar_dot(a, b)
}

#[inline(always)]
pub fn impl_fma(a: F32x4, b: F32x4, c: F32x4) -> F32x4 {
    #[cfg(all(target_arch = "x86_64", target_feature = "fma"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let vb = _mm_loadu_ps(b.lanes.as_ptr());
        let vc = _mm_loadu_ps(c.lanes.as_ptr());
        let vr = _mm_fmadd_ps(va, vb, vc);
        let mut out = F32x4 { lanes: [0.0;4] };
        _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    scalar_fma(a, b, c)
}

#[inline(always)]
pub fn impl_min(a: F32x4, b: F32x4) -> F32x4 {
    #[cfg(all(target_arch = "x86_64", target_feature = "sse"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let vb = _mm_loadu_ps(b.lanes.as_ptr());
        let vr = _mm_min_ps(va, vb);
        let mut out = F32x4 { lanes: [0.0;4] };
        _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    scalar_min(a, b)
}

#[inline(always)]
pub fn impl_max(a: F32x4, b: F32x4) -> F32x4 {
    #[cfg(all(target_arch = "x86_64", target_feature = "sse"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let vb = _mm_loadu_ps(b.lanes.as_ptr());
        let vr = _mm_max_ps(va, vb);
        let mut out = F32x4 { lanes: [0.0;4] };
        _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    scalar_max(a, b)
}

#[inline(always)]
pub fn impl_cmp_lt(a: F32x4, b: F32x4) -> Mask4 {
    #[cfg(all(target_arch = "x86_64", target_feature = "sse"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let vb = _mm_loadu_ps(b.lanes.as_ptr());
        let vr = _mm_cmplt_ps(va, vb);
        let mut tmp = [0.0f32;4];
        _mm_storeu_ps(tmp.as_mut_ptr(), vr);
        let mut r = [0u32;4];
        for i in 0..4 { r[i] = tmp[i].to_bits(); }
        return Mask4 { lanes: r };
    }
    let mut r = [0u32;4];
    for i in 0..4 { r[i] = if a.lanes[i] < b.lanes[i] { 0xFFFF_FFFF } else { 0 } }
    Mask4 { lanes: r }
}

#[inline(always)]
pub fn impl_cmp_eq(a: F32x4, b: F32x4) -> Mask4 {
    #[cfg(all(target_arch = "x86_64", target_feature = "sse"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let vb = _mm_loadu_ps(b.lanes.as_ptr());
        let vr = _mm_cmpeq_ps(va, vb);
        let mut tmp = [0.0f32;4];
        _mm_storeu_ps(tmp.as_mut_ptr(), vr);
        let mut r = [0u32;4];
        for i in 0..4 { r[i] = tmp[i].to_bits(); }
        return Mask4 { lanes: r };
    }
    let mut r = [0u32;4];
    for i in 0..4 { r[i] = if a.lanes[i] == b.lanes[i] { 0xFFFF_FFFF } else { 0 } }
    Mask4 { lanes: r }
}

#[inline(always)]
pub fn impl_select(mask: Mask4, a: F32x4, b: F32x4) -> F32x4 {
    let mut r = [0.0;4];
    for i in 0..4 { r[i] = if mask.lanes[i] != 0 { a.lanes[i] } else { b.lanes[i] } }
    F32x4 { lanes: r }
}

#[inline(always)]
pub fn impl_abs(a: F32x4) -> F32x4 {
    #[cfg(all(target_arch = "x86_64", target_feature = "sse"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFF_FFFF as i32));
        let vr = _mm_and_ps(va, mask);
        let mut out = F32x4 { lanes: [0.0;4] };
        _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    scalar_abs(a)
}
#[inline(always)]
pub fn impl_rcp(a: F32x4) -> F32x4 {
    #[cfg(all(target_arch = "x86_64", target_feature = "sse"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let r0 = _mm_rcp_ps(va);
        let two = _mm_set1_ps(2.0);
        let vr = _mm_mul_ps(r0, _mm_sub_ps(two, _mm_mul_ps(va, r0)));
        let mut out = F32x4 { lanes: [0.0;4] };
        _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    scalar_rcp(a)
}
#[inline(always)]
pub fn impl_rsqrt(a: F32x4) -> F32x4 {
    #[cfg(all(target_arch = "x86_64", target_feature = "sse"))]
    unsafe {
        let va = _mm_loadu_ps(a.lanes.as_ptr());
        let r0 = _mm_rsqrt_ps(va);
        let half = _mm_set1_ps(0.5);
        let three = _mm_set1_ps(3.0);
        let vr = _mm_mul_ps(r0, _mm_sub_ps(three, _mm_mul_ps(_mm_mul_ps(va, r0), _mm_mul_ps(r0, half))));
        let mut out = F32x4 { lanes: [0.0;4] };
        _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    scalar_rsqrt(a)
}
#[inline(always)]
pub fn impl_floor(a: F32x4) -> F32x4 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("sse4.1") {
            let va = _mm_loadu_ps(a.lanes.as_ptr());
            let vr = _mm_floor_ps(va);
            let mut out = F32x4 { lanes: [0.0;4] };
            _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    scalar_floor(a)
}
#[inline(always)]
pub fn impl_ceil(a: F32x4) -> F32x4 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("sse4.1") {
            let va = _mm_loadu_ps(a.lanes.as_ptr());
            let vr = _mm_ceil_ps(va);
            let mut out = F32x4 { lanes: [0.0;4] };
            _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    scalar_ceil(a)
}
#[inline(always)]
pub fn impl_round(a: F32x4) -> F32x4 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("sse4.1") {
            let va = _mm_loadu_ps(a.lanes.as_ptr());
            let vr = _mm_round_ps(va, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
            let mut out = F32x4 { lanes: [0.0;4] };
            _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    scalar_round(a)
}
#[inline(always)]
pub fn impl_trunc(a: F32x4) -> F32x4 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("sse4.1") {
            let va = _mm_loadu_ps(a.lanes.as_ptr());
            let vr = _mm_round_ps(va, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
            let mut out = F32x4 { lanes: [0.0;4] };
            _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    scalar_trunc(a)
}

#[inline(always)]
pub fn impl_and(a: F32x4, b: F32x4) -> F32x4 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("sse") {
            let va = _mm_loadu_ps(a.lanes.as_ptr());
            let vb = _mm_loadu_ps(b.lanes.as_ptr());
            let vr = _mm_and_ps(va, vb);
            let mut out = F32x4 { lanes: [0.0;4] };
            _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    let mut r = [0.0;4]; for i in 0..4 { r[i] = f32::from_bits(a.lanes[i].to_bits() & b.lanes[i].to_bits()) } F32x4 { lanes: r }
}
#[inline(always)]
pub fn impl_or(a: F32x4, b: F32x4) -> F32x4 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("sse") {
            let va = _mm_loadu_ps(a.lanes.as_ptr());
            let vb = _mm_loadu_ps(b.lanes.as_ptr());
            let vr = _mm_or_ps(va, vb);
            let mut out = F32x4 { lanes: [0.0;4] };
            _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    let mut r = [0.0;4]; for i in 0..4 { r[i] = f32::from_bits(a.lanes[i].to_bits() | b.lanes[i].to_bits()) } F32x4 { lanes: r }
}
#[inline(always)]
pub fn impl_xor(a: F32x4, b: F32x4) -> F32x4 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("sse") {
            let va = _mm_loadu_ps(a.lanes.as_ptr());
            let vb = _mm_loadu_ps(b.lanes.as_ptr());
            let vr = _mm_xor_ps(va, vb);
            let mut out = F32x4 { lanes: [0.0;4] };
            _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    let mut r = [0.0;4]; for i in 0..4 { r[i] = f32::from_bits(a.lanes[i].to_bits() ^ b.lanes[i].to_bits()) } F32x4 { lanes: r }
}
#[inline(always)]
pub fn impl_not(a: F32x4) -> F32x4 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("sse") {
            let va = _mm_loadu_ps(a.lanes.as_ptr());
            let mask = _mm_castsi128_ps(_mm_set1_epi32(!0));
            let vr = _mm_xor_ps(va, mask);
            let mut out = F32x4 { lanes: [0.0;4] };
            _mm_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    let mut r = [0.0;4]; for i in 0..4 { r[i] = f32::from_bits(!a.lanes[i].to_bits()) } F32x4 { lanes: r }
}

#[inline(always)]
pub unsafe fn impl_load8(ptr: *const f32) -> F32x8 {
    F32x8 { lanes: [*ptr, *ptr.add(1), *ptr.add(2), *ptr.add(3), *ptr.add(4), *ptr.add(5), *ptr.add(6), *ptr.add(7)] }
}
#[inline(always)]
pub unsafe fn impl_load8_aligned(ptr: *const f32) -> F32x8 {
    #[cfg(all(target_arch = "x86_64", target_feature = "avx"))]
    {
        let r = _mm256_load_ps(ptr);
        let mut out = F32x8 { lanes: [0.0; 8] };
        _mm256_storeu_ps(out.lanes.as_mut_ptr(), r);
        return out;
    }
    impl_load8(ptr)
}
#[inline(always)]
pub unsafe fn impl_store8(ptr: *mut f32, v: F32x8) {
    *ptr = v.lanes[0];
    *ptr.add(1) = v.lanes[1];
    *ptr.add(2) = v.lanes[2];
    *ptr.add(3) = v.lanes[3];
    *ptr.add(4) = v.lanes[4];
    *ptr.add(5) = v.lanes[5];
    *ptr.add(6) = v.lanes[6];
    *ptr.add(7) = v.lanes[7];
}
#[inline(always)]
pub fn impl_add8(a: F32x8, b: F32x8) -> F32x8 {
    #[cfg(all(target_arch = "x86_64", target_feature = "avx"))]
    unsafe {
        let va = _mm256_loadu_ps(a.lanes.as_ptr());
        let vb = _mm256_loadu_ps(b.lanes.as_ptr());
        let vr = _mm256_add_ps(va, vb);
        let mut out = F32x8 { lanes: [0.0;8] };
        _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    let mut r = [0.0;8]; for i in 0..8 { r[i] = a.lanes[i] + b.lanes[i] } F32x8 { lanes: r }
}
#[inline(always)]
pub fn impl_mul8(a: F32x8, b: F32x8) -> F32x8 {
    #[cfg(all(target_arch = "x86_64", target_feature = "avx"))]
    unsafe {
        let va = _mm256_loadu_ps(a.lanes.as_ptr());
        let vb = _mm256_loadu_ps(b.lanes.as_ptr());
        let vr = _mm256_mul_ps(va, vb);
        let mut out = F32x8 { lanes: [0.0;8] };
        _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    let mut r = [0.0;8]; for i in 0..8 { r[i] = a.lanes[i] * b.lanes[i] } F32x8 { lanes: r }
}
#[inline(always)]
pub fn impl_fma8(a: F32x8, b: F32x8, c: F32x8) -> F32x8 {
    #[cfg(all(target_arch = "x86_64", target_feature = "fma"))]
    unsafe {
        let va = _mm256_loadu_ps(a.lanes.as_ptr());
        let vb = _mm256_loadu_ps(b.lanes.as_ptr());
        let vc = _mm256_loadu_ps(c.lanes.as_ptr());
        let vr = _mm256_fmadd_ps(va, vb, vc);
        let mut out = F32x8 { lanes: [0.0;8] };
        _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    #[cfg(all(target_arch = "x86_64", target_feature = "avx", not(target_feature = "fma")))]
    unsafe {
        let va = _mm256_loadu_ps(a.lanes.as_ptr());
        let vb = _mm256_loadu_ps(b.lanes.as_ptr());
        let vc = _mm256_loadu_ps(c.lanes.as_ptr());
        let vr = _mm256_add_ps(_mm256_mul_ps(va, vb), vc);
        let mut out = F32x8 { lanes: [0.0;8] };
        _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    let mut r = [0.0;8]; for i in 0..8 { r[i] = a.lanes[i] * b.lanes[i] + c.lanes[i] } F32x8 { lanes: r }
}

#[inline(always)]
pub fn impl_hsum8(a: F32x8) -> f32 {
    #[cfg(all(target_arch = "x86_64", target_feature = "avx"))]
    unsafe {
        let va = _mm256_loadu_ps(a.lanes.as_ptr());
        let vlow = _mm256_castps256_ps128(va);
        let vhigh = _mm256_extractf128_ps(va, 1);
        let sum = _mm_add_ps(vlow, vhigh);
        let shuf = _mm_movehdup_ps(sum);
        let sums = _mm_add_ps(sum, shuf);
        let shuf2 = _mm_movehl_ps(shuf, sums);
        let sums2 = _mm_add_ss(sums, shuf2);
        return _mm_cvtss_f32(sums2);
    }
    let mut s = 0.0f32; for i in 0..8 { s += a.lanes[i]; } s
}

#[inline(always)]
pub unsafe fn impl_dot3_fma8_aligned(px: *const f32, py: *const f32, pz: *const f32, cx: f32, cy: f32, cz: f32) -> F32x8 {
    #[cfg(all(target_arch = "x86_64", target_feature = "fma"))]
    {
        let vx = _mm256_load_ps(px);
        let vy = _mm256_load_ps(py);
        let vz = _mm256_load_ps(pz);
        let vcx = _mm256_set1_ps(cx);
        let vcy = _mm256_set1_ps(cy);
        let vcz = _mm256_set1_ps(cz);
        let t0 = _mm256_mul_ps(vz, vcz);
        let t1 = _mm256_fmadd_ps(vy, vcy, t0);
        let vr = _mm256_fmadd_ps(vx, vcx, t1);
        let mut out = F32x8 { lanes: [0.0; 8] };
        _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    #[cfg(all(target_arch = "x86_64", target_feature = "avx", not(target_feature = "fma")))]
    {
        let vx = _mm256_load_ps(px);
        let vy = _mm256_load_ps(py);
        let vz = _mm256_load_ps(pz);
        let vcx = _mm256_set1_ps(cx);
        let vcy = _mm256_set1_ps(cy);
        let vcz = _mm256_set1_ps(cz);
        let t0 = _mm256_mul_ps(vz, vcz);
        let t1 = _mm256_add_ps(_mm256_mul_ps(vy, vcy), t0);
        let vr = _mm256_add_ps(_mm256_mul_ps(vx, vcx), t1);
        let mut out = F32x8 { lanes: [0.0; 8] };
        _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
        return out;
    }
    let mut r = [0.0;8];
    for i in 0..8 {
        let x = *px.add(i);
        let y = *py.add(i);
        let z = *pz.add(i);
        r[i] = x * cx + y * cy + z * cz;
    }
    F32x8 { lanes: r }
}
#[inline(always)]
pub fn impl_min8(a: F32x8, b: F32x8) -> F32x8 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("avx") {
            let va = _mm256_loadu_ps(a.lanes.as_ptr());
            let vb = _mm256_loadu_ps(b.lanes.as_ptr());
            let vr = _mm256_min_ps(va, vb);
            let mut out = F32x8 { lanes: [0.0;8] };
            _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    let mut r = [0.0;8]; for i in 0..8 { r[i] = if a.lanes[i] < b.lanes[i] { a.lanes[i] } else { b.lanes[i] } } F32x8 { lanes: r }
}
#[inline(always)]
pub fn impl_max8(a: F32x8, b: F32x8) -> F32x8 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("avx") {
            let va = _mm256_loadu_ps(a.lanes.as_ptr());
            let vb = _mm256_loadu_ps(b.lanes.as_ptr());
            let vr = _mm256_max_ps(va, vb);
            let mut out = F32x8 { lanes: [0.0;8] };
            _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    let mut r = [0.0;8]; for i in 0..8 { r[i] = if a.lanes[i] > b.lanes[i] { a.lanes[i] } else { b.lanes[i] } } F32x8 { lanes: r }
}
#[inline(always)]
pub fn impl_cmp_lt8(a: F32x8, b: F32x8) -> Mask8 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("avx") {
            let va = _mm256_loadu_ps(a.lanes.as_ptr());
            let vb = _mm256_loadu_ps(b.lanes.as_ptr());
            let vr = _mm256_cmp_ps(va, vb, _CMP_LT_OQ);
            let mut tmp = [0.0f32;8];
            _mm256_storeu_ps(tmp.as_mut_ptr(), vr);
            let mut r = [0u32;8];
            for i in 0..8 { r[i] = tmp[i].to_bits(); }
            return Mask8 { lanes: r };
        }
    }
    let mut r = [0u32;8]; for i in 0..8 { r[i] = if a.lanes[i] < b.lanes[i] { 0xFFFF_FFFF } else { 0 } } Mask8 { lanes: r }
}
#[inline(always)]
pub fn impl_cmp_eq8(a: F32x8, b: F32x8) -> Mask8 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("avx") {
            let va = _mm256_loadu_ps(a.lanes.as_ptr());
            let vb = _mm256_loadu_ps(b.lanes.as_ptr());
            let vr = _mm256_cmp_ps(va, vb, _CMP_EQ_OQ);
            let mut tmp = [0.0f32;8];
            _mm256_storeu_ps(tmp.as_mut_ptr(), vr);
            let mut r = [0u32;8];
            for i in 0..8 { r[i] = tmp[i].to_bits(); }
            return Mask8 { lanes: r };
        }
    }
    let mut r = [0u32;8]; for i in 0..8 { r[i] = if a.lanes[i] == b.lanes[i] { 0xFFFF_FFFF } else { 0 } } Mask8 { lanes: r }
}
#[inline(always)]
pub fn impl_select8(mask: Mask8, a: F32x8, b: F32x8) -> F32x8 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("avx") {
            let va = _mm256_loadu_ps(a.lanes.as_ptr());
            let vb = _mm256_loadu_ps(b.lanes.as_ptr());
            let mut mlanes = [0.0f32;8];
            for i in 0..8 { mlanes[i] = f32::from_bits(mask.lanes[i]); }
            let vm = _mm256_loadu_ps(mlanes.as_ptr());
            let t1 = _mm256_and_ps(vm, va);
            let t2 = _mm256_andnot_ps(vm, vb);
            let vr = _mm256_or_ps(t1, t2);
            let mut out = F32x8 { lanes: [0.0;8] };
            _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    let mut r = [0.0;8]; for i in 0..8 { r[i] = if mask.lanes[i] != 0 { a.lanes[i] } else { b.lanes[i] } } F32x8 { lanes: r }
}
#[inline(always)]
pub fn impl_abs8(a: F32x8) -> F32x8 { let mut r = [0.0;8]; for i in 0..8 { r[i] = f32::from_bits(a.lanes[i].to_bits() & 0x7FFF_FFFF) } F32x8 { lanes: r } }
#[inline(always)]
pub fn impl_rcp8(a: F32x8) -> F32x8 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("avx") {
            let va = _mm256_loadu_ps(a.lanes.as_ptr());
            let r0 = _mm256_rcp_ps(va);
            let two = _mm256_set1_ps(2.0);
            let vr = _mm256_mul_ps(r0, _mm256_sub_ps(two, _mm256_mul_ps(va, r0)));
            let mut out = F32x8 { lanes: [0.0;8] };
            _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    let mut r = [0.0;8]; for i in 0..8 { r[i] = 1.0 / a.lanes[i] } F32x8 { lanes: r }
}
#[inline(always)]
pub fn impl_rsqrt8(a: F32x8) -> F32x8 {
    #[cfg(target_arch = "x86_64")]
    unsafe {
        if is_x86_feature_detected!("avx") {
            let va = _mm256_loadu_ps(a.lanes.as_ptr());
            let r0 = _mm256_rsqrt_ps(va);
            let half = _mm256_set1_ps(0.5);
            let three = _mm256_set1_ps(3.0);
            let vr = _mm256_mul_ps(r0, _mm256_sub_ps(three, _mm256_mul_ps(_mm256_mul_ps(va, r0), _mm256_mul_ps(r0, half))));
            let mut out = F32x8 { lanes: [0.0;8] };
            _mm256_storeu_ps(out.lanes.as_mut_ptr(), vr);
            return out;
        }
    }
    let mut r = [0.0;8]; for i in 0..8 { r[i] = 1.0 / a.lanes[i].sqrt() } F32x8 { lanes: r }
}
#[inline(always)]
pub fn impl_floor8(a: F32x8) -> F32x8 { let mut r = [0.0;8]; for i in 0..8 { r[i] = a.lanes[i].floor() } F32x8 { lanes: r } }
#[inline(always)]
pub fn impl_ceil8(a: F32x8) -> F32x8 { let mut r = [0.0;8]; for i in 0..8 { r[i] = a.lanes[i].ceil() } F32x8 { lanes: r } }
#[inline(always)]
pub fn impl_round8(a: F32x8) -> F32x8 { let mut r = [0.0;8]; for i in 0..8 { r[i] = a.lanes[i].round() } F32x8 { lanes: r } }
#[inline(always)]
pub fn impl_trunc8(a: F32x8) -> F32x8 { let mut r = [0.0;8]; for i in 0..8 { r[i] = a.lanes[i].trunc() } F32x8 { lanes: r } }

#[inline(always)]
pub fn impl_and8(a: F32x8, b: F32x8) -> F32x8 { let mut r = [0.0;8]; for i in 0..8 { r[i] = f32::from_bits(a.lanes[i].to_bits() & b.lanes[i].to_bits()) } F32x8 { lanes: r } }
#[inline(always)]
pub fn impl_or8(a: F32x8, b: F32x8) -> F32x8 { let mut r = [0.0;8]; for i in 0..8 { r[i] = f32::from_bits(a.lanes[i].to_bits() | b.lanes[i].to_bits()) } F32x8 { lanes: r } }
#[inline(always)]
pub fn impl_xor8(a: F32x8, b: F32x8) -> F32x8 { let mut r = [0.0;8]; for i in 0..8 { r[i] = f32::from_bits(a.lanes[i].to_bits() ^ b.lanes[i].to_bits()) } F32x8 { lanes: r } }
#[inline(always)]
pub fn impl_not8(a: F32x8) -> F32x8 { let mut r = [0.0;8]; for i in 0..8 { r[i] = f32::from_bits(!a.lanes[i].to_bits()) } F32x8 { lanes: r } }
