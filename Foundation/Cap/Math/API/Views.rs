#[derive(Clone, Copy)]
pub struct Vec3SoAView<'a> {
    pub x: &'a [f32],
    pub y: &'a [f32],
    pub z: &'a [f32],
}

pub struct Vec3SoAMutView<'a> {
    pub x: &'a mut [f32],
    pub y: &'a mut [f32],
    pub z: &'a mut [f32],
}

#[derive(Clone, Copy)]
pub struct Vec4SoAView<'a> {
    pub x: &'a [f32],
    pub y: &'a [f32],
    pub z: &'a [f32],
    pub w: &'a [f32],
}

pub struct Vec4SoAMutView<'a> {
    pub x: &'a mut [f32],
    pub y: &'a mut [f32],
    pub z: &'a mut [f32],
    pub w: &'a mut [f32],
}

#[derive(Clone, Copy)]
pub struct DVec3SoAView<'a> {
    pub x: &'a [f64],
    pub y: &'a [f64],
    pub z: &'a [f64],
}

pub struct DVec3SoAMutView<'a> {
    pub x: &'a mut [f64],
    pub y: &'a mut [f64],
    pub z: &'a mut [f64],
}

impl<'a> Vec3SoAView<'a> {
    pub fn len(&self) -> usize { self.x.len() }
    pub fn validate(&self) -> bool { self.x.len() == self.y.len() && self.y.len() == self.z.len() }
}

impl<'a> Vec3SoAMutView<'a> {
    pub fn len(&self) -> usize { self.x.len() }
    pub fn validate(&self) -> bool { self.x.len() == self.y.len() && self.y.len() == self.z.len() }
}

impl<'a> Vec4SoAView<'a> {
    pub fn len(&self) -> usize { self.x.len() }
    pub fn validate(&self) -> bool { self.x.len() == self.y.len() && self.y.len() == self.z.len() && self.z.len() == self.w.len() }
}

impl<'a> Vec4SoAMutView<'a> {
    pub fn len(&self) -> usize { self.x.len() }
    pub fn validate(&self) -> bool { self.x.len() == self.y.len() && self.y.len() == self.z.len() && self.z.len() == self.w.len() }
}

impl<'a> DVec3SoAView<'a> {
    pub fn len(&self) -> usize { self.x.len() }
    pub fn validate(&self) -> bool { self.x.len() == self.y.len() && self.y.len() == self.z.len() }
}

impl<'a> DVec3SoAMutView<'a> {
    pub fn len(&self) -> usize { self.x.len() }
    pub fn validate(&self) -> bool { self.x.len() == self.y.len() && self.y.len() == self.z.len() }
}
