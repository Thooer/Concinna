mod types;
pub use types::*;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/SIMD.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend {
    use super::*;
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
    pub fn impl_add(a: F32x4, b: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = a.lanes[i] + b.lanes[i]; } F32x4 { lanes: r } }
    pub fn impl_mul(a: F32x4, b: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = a.lanes[i] * b.lanes[i]; } F32x4 { lanes: r } }
    pub fn impl_dot(a: F32x4, b: F32x4) -> f32 { let mut s = 0.0; for i in 0..4 { s += a.lanes[i] * b.lanes[i]; } s }
    pub fn impl_fma(a: F32x4, b: F32x4, c: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = a.lanes[i] * b.lanes[i] + c.lanes[i]; } F32x4 { lanes: r } }
    pub fn impl_min(a: F32x4, b: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = if a.lanes[i] < b.lanes[i] { a.lanes[i] } else { b.lanes[i] } } F32x4 { lanes: r } }
    pub fn impl_max(a: F32x4, b: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = if a.lanes[i] > b.lanes[i] { a.lanes[i] } else { b.lanes[i] } } F32x4 { lanes: r } }
    pub fn impl_cmp_lt(a: F32x4, b: F32x4) -> Mask4 { let mut r = [0u32;4]; for i in 0..4 { r[i] = if a.lanes[i] < b.lanes[i] { 0xFFFF_FFFF } else { 0 } } Mask4 { lanes: r } }
    pub fn impl_cmp_eq(a: F32x4, b: F32x4) -> Mask4 { let mut r = [0u32;4]; for i in 0..4 { r[i] = if a.lanes[i] == b.lanes[i] { 0xFFFF_FFFF } else { 0 } } Mask4 { lanes: r } }
    pub fn impl_select(mask: Mask4, a: F32x4, b: F32x4) -> F32x4 { let mut r = [0.0;4]; for i in 0..4 { r[i] = if mask.lanes[i] != 0 { a.lanes[i] } else { b.lanes[i] } } F32x4 { lanes: r } }
    #[inline(always)]
    pub unsafe fn impl_load8(ptr: *const f32) -> F32x8 {
        F32x8 { lanes: [*ptr, *ptr.add(1), *ptr.add(2), *ptr.add(3), *ptr.add(4), *ptr.add(5), *ptr.add(6), *ptr.add(7)] }
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
    pub unsafe fn impl_load8_aligned(ptr: *const f32) -> F32x8 {
        F32x8 { lanes: [*ptr, *ptr.add(1), *ptr.add(2), *ptr.add(3), *ptr.add(4), *ptr.add(5), *ptr.add(6), *ptr.add(7)] }
    }
    pub fn impl_hsum8(a: F32x8) -> f32 { let mut s = 0.0; for i in 0..8 { s += a.lanes[i]; } s }
}

pub unsafe fn load(ptr: *const f32) -> F32x4 { backend::impl_load(ptr) }
pub unsafe fn store(ptr: *mut f32, v: F32x4) { backend::impl_store(ptr, v) }
pub fn add(a: F32x4, b: F32x4) -> F32x4 { backend::impl_add(a, b) }
pub fn mul(a: F32x4, b: F32x4) -> F32x4 { backend::impl_mul(a, b) }
pub fn dot(a: F32x4, b: F32x4) -> f32 { backend::impl_dot(a, b) }
pub fn fma(a: F32x4, b: F32x4, c: F32x4) -> F32x4 { backend::impl_fma(a, b, c) }
pub fn min(a: F32x4, b: F32x4) -> F32x4 { backend::impl_min(a, b) }
pub fn max(a: F32x4, b: F32x4) -> F32x4 { backend::impl_max(a, b) }
pub fn cmp_lt(a: F32x4, b: F32x4) -> Mask4 { backend::impl_cmp_lt(a, b) }
pub fn cmp_eq(a: F32x4, b: F32x4) -> Mask4 { backend::impl_cmp_eq(a, b) }
pub fn select(mask: Mask4, a: F32x4, b: F32x4) -> F32x4 { backend::impl_select(mask, a, b) }

pub unsafe fn load8(ptr: *const f32) -> F32x8 { backend::impl_load8(ptr) }
pub unsafe fn load8_aligned(ptr: *const f32) -> F32x8 { backend::impl_load8_aligned(ptr) }
pub unsafe fn store8(ptr: *mut f32, v: F32x8) { backend::impl_store8(ptr, v) }
pub fn add8(a: F32x8, b: F32x8) -> F32x8 { backend::impl_add8(a, b) }
pub fn mul8(a: F32x8, b: F32x8) -> F32x8 { backend::impl_mul8(a, b) }
pub fn fma8(a: F32x8, b: F32x8, c: F32x8) -> F32x8 { backend::impl_fma8(a, b, c) }
pub fn min8(a: F32x8, b: F32x8) -> F32x8 { backend::impl_min8(a, b) }
pub fn max8(a: F32x8, b: F32x8) -> F32x8 { backend::impl_max8(a, b) }
pub fn cmp_lt8(a: F32x8, b: F32x8) -> Mask8 { backend::impl_cmp_lt8(a, b) }
pub fn cmp_eq8(a: F32x8, b: F32x8) -> Mask8 { backend::impl_cmp_eq8(a, b) }
pub fn select8(mask: Mask8, a: F32x8, b: F32x8) -> F32x8 { backend::impl_select8(mask, a, b) }

pub fn abs(a: F32x4) -> F32x4 { backend::impl_abs(a) }
pub fn abs8(a: F32x8) -> F32x8 { backend::impl_abs8(a) }
pub fn rcp(a: F32x4) -> F32x4 { backend::impl_rcp(a) }
pub fn rcp8(a: F32x8) -> F32x8 { backend::impl_rcp8(a) }
pub fn rsqrt(a: F32x4) -> F32x4 { backend::impl_rsqrt(a) }
pub fn rsqrt8(a: F32x8) -> F32x8 { backend::impl_rsqrt8(a) }
pub fn floor(a: F32x4) -> F32x4 { backend::impl_floor(a) }
pub fn floor8(a: F32x8) -> F32x8 { backend::impl_floor8(a) }
pub fn ceil(a: F32x4) -> F32x4 { backend::impl_ceil(a) }
pub fn ceil8(a: F32x8) -> F32x8 { backend::impl_ceil8(a) }
pub fn round(a: F32x4) -> F32x4 { backend::impl_round(a) }
pub fn round8(a: F32x8) -> F32x8 { backend::impl_round8(a) }
pub fn trunc8(a: F32x8) -> F32x8 { backend::impl_trunc8(a) }

pub fn and(a: F32x4, b: F32x4) -> F32x4 { backend::impl_and(a, b) }
pub fn or(a: F32x4, b: F32x4) -> F32x4 { backend::impl_or(a, b) }
pub fn xor(a: F32x4, b: F32x4) -> F32x4 { backend::impl_xor(a, b) }
pub fn not(a: F32x4) -> F32x4 { backend::impl_not(a) }
pub fn and8(a: F32x8, b: F32x8) -> F32x8 { backend::impl_and8(a, b) }
pub fn or8(a: F32x8, b: F32x8) -> F32x8 { backend::impl_or8(a, b) }
pub fn xor8(a: F32x8, b: F32x8) -> F32x8 { backend::impl_xor8(a, b) }
pub fn not8(a: F32x8) -> F32x8 { backend::impl_not8(a) }

pub unsafe fn dot3_fma8_aligned(px: *const f32, py: *const f32, pz: *const f32, cx: f32, cy: f32, cz: f32) -> F32x8 { backend::impl_dot3_fma8_aligned(px, py, pz, cx, cy, cz) }
pub fn hsum8(a: F32x8) -> f32 { backend::impl_hsum8(a) }
