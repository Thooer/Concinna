use prm_simd::*;

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Vec2 { pub x: f32, pub y: f32 }

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Vec3 { pub x: f32, pub y: f32, pub z: f32 }

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Vec4 { pub x: f32, pub y: f32, pub z: f32, pub w: f32 }

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct DVec3 { pub x: f64, pub y: f64, pub z: f64 }

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Quat { pub x: f32, pub y: f32, pub z: f32, pub w: f32 }

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Mat4 { pub rows: [[f32; 4]; 4] }

#[derive(Clone, Debug, PartialEq)]
pub struct Vec3SoA { pub xs: Vec<f32>, pub ys: Vec<f32>, pub zs: Vec<f32> }

#[derive(Clone, Debug, PartialEq)]
pub struct Vec4SoA { pub xs: Vec<f32>, pub ys: Vec<f32>, pub zs: Vec<f32>, pub ws: Vec<f32> }

impl Vec2 {
    pub fn new(x: f32, y: f32) -> Self { Self { x, y } }
    pub fn dot(self, o: Self) -> f32 { self.x * o.x + self.y * o.y }
    pub fn length(self) -> f32 { (self.dot(self)).sqrt() }
    pub fn normalize(self) -> Self { let l = self.length(); if l > 0.0 { Self { x: self.x / l, y: self.y / l } } else { self } }
}

impl Vec3 {
    pub fn new(x: f32, y: f32, z: f32) -> Self { Self { x, y, z } }
    pub fn add(self, o: Self) -> Self { Self { x: self.x + o.x, y: self.y + o.y, z: self.z + o.z } }
    pub fn sub(self, o: Self) -> Self { Self { x: self.x - o.x, y: self.y - o.y, z: self.z - o.z } }
    pub fn dot(self, o: Self) -> f32 { self.x * o.x + self.y * o.y + self.z * o.z }
    pub fn cross(self, o: Self) -> Self { Self { x: self.y * o.z - self.z * o.y, y: self.z * o.x - self.x * o.z, z: self.x * o.y - self.y * o.x } }
    pub fn length(self) -> f32 { (self.dot(self)).sqrt() }
    pub fn normalize(self) -> Self { let l = self.length(); if l > 0.0 { Self { x: self.x / l, y: self.y / l, z: self.z / l } } else { self } }
}

impl Vec4 {
    pub fn new(x: f32, y: f32, z: f32, w: f32) -> Self { Self { x, y, z, w } }
    pub fn add(self, o: Self) -> Self { Self { x: self.x + o.x, y: self.y + o.y, z: self.z + o.z, w: self.w + o.w } }
    pub fn sub(self, o: Self) -> Self { Self { x: self.x - o.x, y: self.y - o.y, z: self.z - o.z, w: self.w - o.w } }
    pub fn dot(self, o: Self) -> f32 { self.x * o.x + self.y * o.y + self.z * o.z + self.w * o.w }
    pub fn length(self) -> f32 { (self.dot(self)).sqrt() }
    pub fn normalize(self) -> Self { let l = self.length(); if l > 0.0 { Self { x: self.x / l, y: self.y / l, z: self.z / l, w: self.w / l } } else { self } }
}

impl DVec3 {
    pub fn new(x: f64, y: f64, z: f64) -> Self { Self { x, y, z } }
}

impl Quat {
    pub fn new(x: f32, y: f32, z: f32, w: f32) -> Self { Self { x, y, z, w } }
    pub fn identity() -> Self { Self { x: 0.0, y: 0.0, z: 0.0, w: 1.0 } }
    pub fn mul(self, o: Self) -> Self {
        Self {
            w: self.w * o.w - self.x * o.x - self.y * o.y - self.z * o.z,
            x: self.w * o.x + self.x * o.w + self.y * o.z - self.z * o.y,
            y: self.w * o.y - self.x * o.z + self.y * o.w + self.z * o.x,
            z: self.w * o.z + self.x * o.y - self.y * o.x + self.z * o.w,
        }
    }
    pub fn normalize(self) -> Self {
        let l = (self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w).sqrt();
        if l > 0.0 { Self { x: self.x / l, y: self.y / l, z: self.z / l, w: self.w / l } } else { self }
    }
    pub fn from_axis_angle(axis: Vec3, angle: f32) -> Self {
        let h = 0.5 * angle;
        let s = h.sin();
        let a = axis.normalize();
        Self { x: a.x * s, y: a.y * s, z: a.z * s, w: h.cos() }
    }
}

impl Mat4 {
    pub fn identity() -> Self { Self { rows: [[1.0,0.0,0.0,0.0],[0.0,1.0,0.0,0.0],[0.0,0.0,1.0,0.0],[0.0,0.0,0.0,1.0]] } }
    pub fn mul(self, o: Self) -> Self {
        let mut r = [[0.0;4];4];
        
        unsafe {
            // 遍历第一个矩阵的每一行
            for i in 0..4 {
                // 加载第一个矩阵的当前行
                let a_row = F32x4 { lanes: self.rows[i] };
                
                // 加载第二个矩阵的四行
                let b_row0 = F32x4 { lanes: o.rows[0] };
                let b_row1 = F32x4 { lanes: o.rows[1] };
                let b_row2 = F32x4 { lanes: o.rows[2] };
                let b_row3 = F32x4 { lanes: o.rows[3] };
                
                // 计算结果行的第一个元素
                let a0 = F32x4 { lanes: [a_row.lanes[0], a_row.lanes[0], a_row.lanes[0], a_row.lanes[0]] };
                let t0 = mul(a0, b_row0);
                
                let a1 = F32x4 { lanes: [a_row.lanes[1], a_row.lanes[1], a_row.lanes[1], a_row.lanes[1]] };
                let t1 = fma(a1, b_row1, t0);
                
                let a2 = F32x4 { lanes: [a_row.lanes[2], a_row.lanes[2], a_row.lanes[2], a_row.lanes[2]] };
                let t2 = fma(a2, b_row2, t1);
                
                let a3 = F32x4 { lanes: [a_row.lanes[3], a_row.lanes[3], a_row.lanes[3], a_row.lanes[3]] };
                let res = fma(a3, b_row3, t2);
                
                // 存储结果行
                store(r[i].as_mut_ptr(), res);
            }
        }
        
        Self { rows: r }
    }
    pub fn transpose(self) -> Self {
        let mut r = [[0.0;4];4];
        for i in 0..4 { for j in 0..4 { r[i][j] = self.rows[j][i]; } }
        Self { rows: r }
    }
    pub fn transform_point(self, v: Vec3) -> Vec3 {
        let x = v.x * self.rows[0][0] + v.y * self.rows[0][1] + v.z * self.rows[0][2] + self.rows[0][3];
        let y = v.x * self.rows[1][0] + v.y * self.rows[1][1] + v.z * self.rows[1][2] + self.rows[1][3];
        let z = v.x * self.rows[2][0] + v.y * self.rows[2][1] + v.z * self.rows[2][2] + self.rows[2][3];
        Vec3 { x, y, z }
    }
    pub fn from_trs(t: Vec3, r: Quat, s: Vec3) -> Self {
        let r = r.normalize();
        let x2 = r.x + r.x; let y2 = r.y + r.y; let z2 = r.z + r.z;
        let xx = r.x * x2; let yy = r.y * y2; let zz = r.z * z2;
        let xy = r.x * y2; let xz = r.x * z2; let yz = r.y * z2;
        let wx = r.w * x2; let wy = r.w * y2; let wz = r.w * z2;
        let m00 = (1.0 - (yy + zz)) * s.x; let m01 = (xy + wz) * s.x; let m02 = (xz - wy) * s.x;
        let m10 = (xy - wz) * s.y; let m11 = (1.0 - (xx + zz)) * s.y; let m12 = (yz + wx) * s.y;
        let m20 = (xz + wy) * s.z; let m21 = (yz - wx) * s.z; let m22 = (1.0 - (xx + yy)) * s.z;
        Self { rows: [[m00,m01,m02,t.x],[m10,m11,m12,t.y],[m20,m21,m22,t.z],[0.0,0.0,0.0,1.0]] }
    }
    pub fn inverse_affine(self) -> Self {
        let r0 = self.rows[0]; let r1 = self.rows[1]; let r2 = self.rows[2];
        let t = Vec3 { x: self.rows[0][3], y: self.rows[1][3], z: self.rows[2][3] };
        let det = r0[0]*(r1[1]*r2[2]-r1[2]*r2[1]) - r0[1]*(r1[0]*r2[2]-r1[2]*r2[0]) + r0[2]*(r1[0]*r2[1]-r1[1]*r2[0]);
        let inv_det = 1.0/det;
        let m00 = (r1[1]*r2[2]-r1[2]*r2[1])*inv_det;
        let m01 = (r0[2]*r2[1]-r0[1]*r2[2])*inv_det;
        let m02 = (r0[1]*r1[2]-r0[2]*r1[1])*inv_det;
        let m10 = (r1[2]*r2[0]-r1[0]*r2[2])*inv_det;
        let m11 = (r0[0]*r2[2]-r0[2]*r2[0])*inv_det;
        let m12 = (r0[2]*r1[0]-r0[0]*r1[2])*inv_det;
        let m20 = (r1[0]*r2[1]-r1[1]*r2[0])*inv_det;
        let m21 = (r0[1]*r2[0]-r0[0]*r2[1])*inv_det;
        let m22 = (r0[0]*r1[1]-r0[1]*r1[0])*inv_det;
        let itx = -(t.x*m00 + t.y*m01 + t.z*m02);
        let ity = -(t.x*m10 + t.y*m11 + t.z*m12);
        let itz = -(t.x*m20 + t.y*m21 + t.z*m22);
        Self { rows: [[m00,m01,m02,itx],[m10,m11,m12,ity],[m20,m21,m22,itz],[0.0,0.0,0.0,1.0]] }
    }
}

impl Vec3SoA {
    pub fn with_len(n: usize) -> Self { Self { xs: vec![0.0; n], ys: vec![0.0; n], zs: vec![0.0; n] } }
    pub fn len(&self) -> usize { self.xs.len().min(self.ys.len()).min(self.zs.len()) }
    pub fn from_aos(a: &[Vec3]) -> Self {
        let n = a.len();
        let mut r = Self::with_len(n);
        for i in 0..n { r.xs[i] = a[i].x; r.ys[i] = a[i].y; r.zs[i] = a[i].z; }
        r
    }
    pub fn to_aos(&self) -> Vec<Vec3> {
        let n = self.len();
        let mut out = Vec::with_capacity(n);
        for i in 0..n { out.push(Vec3 { x: self.xs[i], y: self.ys[i], z: self.zs[i] }); }
        out
    }
}

impl Vec4SoA {
    pub fn with_len(n: usize) -> Self { Self { xs: vec![0.0; n], ys: vec![0.0; n], zs: vec![0.0; n], ws: vec![0.0; n] } }
    pub fn len(&self) -> usize { self.xs.len().min(self.ys.len()).min(self.zs.len()).min(self.ws.len()) }
    pub fn from_aos(a: &[Vec4]) -> Self {
        let n = a.len();
        let mut r = Self::with_len(n);
        for i in 0..n { r.xs[i] = a[i].x; r.ys[i] = a[i].y; r.zs[i] = a[i].z; r.ws[i] = a[i].w; }
        r
    }
    pub fn to_aos(&self) -> Vec<Vec4> {
        let n = self.len();
        let mut out = Vec::with_capacity(n);
        for i in 0..n { out.push(Vec4 { x: self.xs[i], y: self.ys[i], z: self.zs[i], w: self.ws[i] }); }
        out
    }
}

impl Rect {
    pub fn new(min: Vec2, max: Vec2) -> Self { Self { min, max } }
}

impl AABB {
    pub fn new(min: Vec3, max: Vec3) -> Self { Self { min, max } }
    pub fn from_center_extent(center: Vec3, extent: Vec3) -> Self { Self { min: Vec3 { x: center.x - extent.x, y: center.y - extent.y, z: center.z - extent.z }, max: Vec3 { x: center.x + extent.x, y: center.y + extent.y, z: center.z + extent.z } } }
    pub fn center(self) -> Vec3 { Vec3 { x: (self.min.x + self.max.x)*0.5, y: (self.min.y + self.max.y)*0.5, z: (self.min.z + self.max.z)*0.5 } }
    pub fn extent(self) -> Vec3 { Vec3 { x: (self.max.x - self.min.x)*0.5, y: (self.max.y - self.min.y)*0.5, z: (self.max.z - self.min.z)*0.5 } }
}

impl Plane {
    pub fn new(n: Vec3, d: f32) -> Self { Self { n, d } }
    pub fn from_point_normal(p: Vec3, n: Vec3) -> Self { let nn = n.normalize(); Self { n: nn, d: -nn.dot(p) } }
    pub fn normalize(self) -> Self { let l = self.n.length(); if l > 0.0 { Self { n: Vec3 { x: self.n.x/l, y: self.n.y/l, z: self.n.z/l }, d: self.d/l } } else { self } }
}

impl Frustum { pub fn new(planes: [Plane;6]) -> Self { Self { planes } } }
impl Ray { pub fn new(o: Vec3, dir: Vec3) -> Self { Self { o, dir } } pub fn at(self, t: f32) -> Vec3 { Vec3 { x: self.o.x + self.dir.x * t, y: self.o.y + self.dir.y * t, z: self.o.z + self.dir.z * t } } }
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Rect { pub min: Vec2, pub max: Vec2 }

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct AABB { pub min: Vec3, pub max: Vec3 }

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Plane { pub n: Vec3, pub d: f32 }

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Frustum { pub planes: [Plane; 6] }

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Ray { pub o: Vec3, pub dir: Vec3 }
