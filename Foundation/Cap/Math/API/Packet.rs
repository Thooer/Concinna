use prm_simd::{F32x4, F32x8, load, store, add, mul, fma, load8, store8, add8, mul8, fma8};

#[derive(Clone, Copy)]
pub struct Packet4 { pub reg: F32x4 }

impl Packet4 {
    #[inline(always)]
    pub unsafe fn load(ptr: *const f32) -> Self { Self { reg: load(ptr) } }
    #[inline(always)]
    pub unsafe fn store(self, ptr: *mut f32) { store(ptr, self.reg); }
    #[inline(always)]
    pub fn add(self, other: Self) -> Self { Self { reg: add(self.reg, other.reg) } }
    #[inline(always)]
    pub fn mul(self, other: Self) -> Self { Self { reg: mul(self.reg, other.reg) } }
    #[inline(always)]
    pub fn fma(self, other: Self, acc: Self) -> Self { Self { reg: fma(self.reg, other.reg, acc.reg) } }
}

#[derive(Clone, Copy)]
pub struct Packet8 { pub reg: F32x8 }

impl Packet8 {
    #[inline(always)]
    pub unsafe fn load(ptr: *const f32) -> Self { Self { reg: load8(ptr) } }
    #[inline(always)]
    pub unsafe fn store(self, ptr: *mut f32) { store8(ptr, self.reg); }
    #[inline(always)]
    pub fn add(self, rhs: Self) -> Self { Self { reg: add8(self.reg, rhs.reg) } }
    #[inline(always)]
    pub fn mul(self, rhs: Self) -> Self { Self { reg: mul8(self.reg, rhs.reg) } }
    #[inline(always)]
    pub fn fma(self, other: Self, acc: Self) -> Self { Self { reg: fma8(self.reg, other.reg, acc.reg) } }
}
