use cap_math::{Vec3, Quat, Mat4};
use cap_containers::{SoA, SoAColumn, UseVector};

#[derive(Clone, Copy)]
pub struct Transform {
    pub px: f32, pub py: f32, pub pz: f32,
    pub rx: f32, pub ry: f32, pub rz: f32, pub rw: f32,
    pub sx: f32, pub sy: f32, pub sz: f32,
}

impl Transform {
    pub fn from_trs(p: Vec3, r: Quat, s: Vec3) -> Self {
        Self { px: p.x, py: p.y, pz: p.z, rx: r.x, ry: r.y, rz: r.z, rw: r.w, sx: s.x, sy: s.y, sz: s.z }
    }
    pub fn position(&self) -> Vec3 { Vec3 { x: self.px, y: self.py, z: self.pz } }
    pub fn rotation(&self) -> Quat { Quat { x: self.rx, y: self.ry, z: self.rz, w: self.rw } }
    pub fn scale(&self) -> Vec3 { Vec3 { x: self.sx, y: self.sy, z: self.sz } }
    pub fn model_matrix(&self) -> Mat4 { Mat4::from_trs(self.position(), self.rotation(), self.scale()) }
}

pub struct TransformSoA<'a, M = UseVector>
where
    f32: SoA<M>,
{
    pub px: <f32 as SoA<M>>::Storage<'a>,
    pub py: <f32 as SoA<M>>::Storage<'a>,
    pub pz: <f32 as SoA<M>>::Storage<'a>,
    pub rx: <f32 as SoA<M>>::Storage<'a>,
    pub ry: <f32 as SoA<M>>::Storage<'a>,
    pub rz: <f32 as SoA<M>>::Storage<'a>,
    pub rw: <f32 as SoA<M>>::Storage<'a>,
    pub sx: <f32 as SoA<M>>::Storage<'a>,
    pub sy: <f32 as SoA<M>>::Storage<'a>,
    pub sz: <f32 as SoA<M>>::Storage<'a>,
}

impl<'a, M> TransformSoA<'a, M>
where
    f32: SoA<M>,
{
    pub fn with_capacity(alloc: cap_memory::Allocator<'a>, capacity: usize) -> Result<Self, cap_memory::MemoryError> {
        Ok(Self {
            px: <<f32 as SoA<M>>::Storage<'a> as SoAColumn<'a, f32>>::with_capacity(alloc, capacity)?,
            py: <<f32 as SoA<M>>::Storage<'a> as SoAColumn<'a, f32>>::with_capacity(alloc, capacity)?,
            pz: <<f32 as SoA<M>>::Storage<'a> as SoAColumn<'a, f32>>::with_capacity(alloc, capacity)?,
            rx: <<f32 as SoA<M>>::Storage<'a> as SoAColumn<'a, f32>>::with_capacity(alloc, capacity)?,
            ry: <<f32 as SoA<M>>::Storage<'a> as SoAColumn<'a, f32>>::with_capacity(alloc, capacity)?,
            rz: <<f32 as SoA<M>>::Storage<'a> as SoAColumn<'a, f32>>::with_capacity(alloc, capacity)?,
            rw: <<f32 as SoA<M>>::Storage<'a> as SoAColumn<'a, f32>>::with_capacity(alloc, capacity)?,
            sx: <<f32 as SoA<M>>::Storage<'a> as SoAColumn<'a, f32>>::with_capacity(alloc, capacity)?,
            sy: <<f32 as SoA<M>>::Storage<'a> as SoAColumn<'a, f32>>::with_capacity(alloc, capacity)?,
            sz: <<f32 as SoA<M>>::Storage<'a> as SoAColumn<'a, f32>>::with_capacity(alloc, capacity)?,
        })
    }

    pub fn push(&mut self, value: Transform) -> Result<(), cap_memory::MemoryError> {
        <_ as SoAColumn<'a, f32>>::push(&mut self.px, value.px)?;
        <_ as SoAColumn<'a, f32>>::push(&mut self.py, value.py)?;
        <_ as SoAColumn<'a, f32>>::push(&mut self.pz, value.pz)?;
        <_ as SoAColumn<'a, f32>>::push(&mut self.rx, value.rx)?;
        <_ as SoAColumn<'a, f32>>::push(&mut self.ry, value.ry)?;
        <_ as SoAColumn<'a, f32>>::push(&mut self.rz, value.rz)?;
        <_ as SoAColumn<'a, f32>>::push(&mut self.rw, value.rw)?;
        <_ as SoAColumn<'a, f32>>::push(&mut self.sx, value.sx)?;
        <_ as SoAColumn<'a, f32>>::push(&mut self.sy, value.sy)?;
        <_ as SoAColumn<'a, f32>>::push(&mut self.sz, value.sz)?;
        Ok(())
    }

    pub fn len(&self) -> usize {
        SoAColumn::<f32>::len(&self.px)
    }
}

impl<'a> TransformSoA<'a, UseVector> {
    pub fn set_position(&mut self, i: usize, p: Vec3) {
        if i < self.px.len() {
            let sx = self.px.as_mut_slice();
            let sy = self.py.as_mut_slice();
            let sz = self.pz.as_mut_slice();
            sx[i] = p.x; sy[i] = p.y; sz[i] = p.z;
        }
    }
    pub fn get_position(&self, i: usize) -> Option<Vec3> {
        if i < self.px.len() {
            let x = SoAColumn::<f32>::get(&self.px, i).unwrap_or(0.0);
            let y = SoAColumn::<f32>::get(&self.py, i).unwrap_or(0.0);
            let z = SoAColumn::<f32>::get(&self.pz, i).unwrap_or(0.0);
            Some(Vec3 { x, y, z })
        } else { None }
    }
}
