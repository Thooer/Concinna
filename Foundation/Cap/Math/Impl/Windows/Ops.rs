use crate::*;
use prm_simd::*;
use crate::packet::{Packet4, Packet8};
use crate::{Vec3SoAView, Vec3SoAMutView};
use std::ptr;

pub fn impl_vec4_add(a: Vec4, b: Vec4) -> Vec4 {
    let va = F32x4 { lanes: [a.x, a.y, a.z, a.w] };
    let vb = F32x4 { lanes: [b.x, b.y, b.z, b.w] };
    let r = add(va, vb);
    Vec4 { x: r.lanes[0], y: r.lanes[1], z: r.lanes[2], w: r.lanes[3] }
}

pub fn impl_vec4_add_batch(a: &[[f32;4]], b: &[[f32;4]], out: &mut [[f32;4]]) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    for i in 0..n {
        let va = F32x4 { lanes: a[i] };
        let vb = F32x4 { lanes: b[i] };
        let r = add(va, vb);
        out[i] = r.lanes;
    }
    n
}

pub fn impl_mat4_mul(a: Mat4, b: Mat4) -> Mat4 {
    let bt = b.transpose();
    let mut r = [[0.0;4];4];
    for i in 0..4 {
        let ai = F32x4 { lanes: a.rows[i] };
        for j in 0..4 {
            let bj = F32x4 { lanes: bt.rows[j] };
            let m = mul(ai, bj);
            r[i][j] = m.lanes[0] + m.lanes[1] + m.lanes[2] + m.lanes[3];
        }
    }
    Mat4 { rows: r }
}

pub fn impl_vec4_add_soa(a: &Vec4SoA, b: &Vec4SoA, out: &mut Vec4SoA) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    let mut i = 0usize;
    while i + 8 <= n {
        unsafe {
            let ax = Packet8::load(a.xs.as_ptr().add(i));
            let bx = Packet8::load(b.xs.as_ptr().add(i));
            ax.add(bx).store(out.xs.as_mut_ptr().add(i));

            let ay = Packet8::load(a.ys.as_ptr().add(i));
            let by = Packet8::load(b.ys.as_ptr().add(i));
            ay.add(by).store(out.ys.as_mut_ptr().add(i));

            let az = Packet8::load(a.zs.as_ptr().add(i));
            let bz = Packet8::load(b.zs.as_ptr().add(i));
            az.add(bz).store(out.zs.as_mut_ptr().add(i));

            let aw = Packet8::load(a.ws.as_ptr().add(i));
            let bw = Packet8::load(b.ws.as_ptr().add(i));
            aw.add(bw).store(out.ws.as_mut_ptr().add(i));
        }
        i += 8;
    }
    while i + 4 <= n {
        unsafe {
            let ax = Packet4::load(a.xs.as_ptr().add(i));
            let bx = Packet4::load(b.xs.as_ptr().add(i));
            ax.add(bx).store(out.xs.as_mut_ptr().add(i));

            let ay = Packet4::load(a.ys.as_ptr().add(i));
            let by = Packet4::load(b.ys.as_ptr().add(i));
            ay.add(by).store(out.ys.as_mut_ptr().add(i));

            let az = Packet4::load(a.zs.as_ptr().add(i));
            let bz = Packet4::load(b.zs.as_ptr().add(i));
            az.add(bz).store(out.zs.as_mut_ptr().add(i));

            let aw = Packet4::load(a.ws.as_ptr().add(i));
            let bw = Packet4::load(b.ws.as_ptr().add(i));
            aw.add(bw).store(out.ws.as_mut_ptr().add(i));
        }
        i += 4;
    }
    while i < n {
        out.xs[i] = a.xs[i] + b.xs[i];
        out.ys[i] = a.ys[i] + b.ys[i];
        out.zs[i] = a.zs[i] + b.zs[i];
        out.ws[i] = a.ws[i] + b.ws[i];
        i += 1;
    }
    n
}

pub fn impl_vec3_dot_soa(a: &Vec3SoA, b: &Vec3SoA, out: &mut [f32]) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    let mut i = 0usize;
    while i + 8 <= n {
        unsafe {
            let ax8 = Packet8::load(a.xs.as_ptr().add(i));
            let bx8 = Packet8::load(b.xs.as_ptr().add(i));
            let ay8 = Packet8::load(a.ys.as_ptr().add(i));
            let by8 = Packet8::load(b.ys.as_ptr().add(i));
            let az8 = Packet8::load(a.zs.as_ptr().add(i));
            let bz8 = Packet8::load(b.zs.as_ptr().add(i));
            let mx = mul8(ax8.reg, bx8.reg);
            let my = mul8(ay8.reg, by8.reg);
            let mz = mul8(az8.reg, bz8.reg);
            let sum = add8(mx, add8(my, mz));
            let _ = store8(out.as_mut_ptr().add(i), sum);
        }
        i += 8;
    }
    while i + 4 <= n {
        unsafe {
            let ax = Packet4::load(a.xs.as_ptr().add(i));
            let bx = Packet4::load(b.xs.as_ptr().add(i));
            let mx = mul(ax.reg, bx.reg);

            let ay = Packet4::load(a.ys.as_ptr().add(i));
            let by = Packet4::load(b.ys.as_ptr().add(i));
            let my = mul(ay.reg, by.reg);

            let az = Packet4::load(a.zs.as_ptr().add(i));
            let bz = Packet4::load(b.zs.as_ptr().add(i));
            let mz = mul(az.reg, bz.reg);

            let sum = F32x4 { lanes: [mx.lanes[0]+my.lanes[0]+mz.lanes[0], mx.lanes[1]+my.lanes[1]+mz.lanes[1], mx.lanes[2]+my.lanes[2]+mz.lanes[2], mx.lanes[3]+my.lanes[3]+mz.lanes[3]] };
            ptr::copy_nonoverlapping(sum.lanes.as_ptr(), out.as_mut_ptr().add(i), 4);
        }
        i += 4;
    }
    while i < n {
        out[i] = a.xs[i]*b.xs[i] + a.ys[i]*b.ys[i] + a.zs[i]*b.zs[i];
        i += 1;
    }
    n
}

pub fn impl_vec4_add_soa_unchecked(a: &Vec4SoA, b: &Vec4SoA, out: &mut Vec4SoA) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    let mut i = 0usize;
    unsafe {
        while i + 8 <= n {
            Packet8::load(a.xs.as_ptr().add(i)).add(Packet8::load(b.xs.as_ptr().add(i))).store(out.xs.as_mut_ptr().add(i));
            Packet8::load(a.ys.as_ptr().add(i)).add(Packet8::load(b.ys.as_ptr().add(i))).store(out.ys.as_mut_ptr().add(i));
            Packet8::load(a.zs.as_ptr().add(i)).add(Packet8::load(b.zs.as_ptr().add(i))).store(out.zs.as_mut_ptr().add(i));
            Packet8::load(a.ws.as_ptr().add(i)).add(Packet8::load(b.ws.as_ptr().add(i))).store(out.ws.as_mut_ptr().add(i));
            i += 8;
        }
        while i + 4 <= n {
            Packet4::load(a.xs.as_ptr().add(i)).add(Packet4::load(b.xs.as_ptr().add(i))).store(out.xs.as_mut_ptr().add(i));
            Packet4::load(a.ys.as_ptr().add(i)).add(Packet4::load(b.ys.as_ptr().add(i))).store(out.ys.as_mut_ptr().add(i));
            Packet4::load(a.zs.as_ptr().add(i)).add(Packet4::load(b.zs.as_ptr().add(i))).store(out.zs.as_mut_ptr().add(i));
            Packet4::load(a.ws.as_ptr().add(i)).add(Packet4::load(b.ws.as_ptr().add(i))).store(out.ws.as_mut_ptr().add(i));
            i += 4;
        }
    }
    n
}

pub fn impl_vec3_dot_soa_unchecked(a: &Vec3SoA, b: &Vec3SoA, out: &mut [f32]) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    let mut i = 0usize;
    unsafe {
        while i + 4 <= n {
            let mx = mul(Packet4::load(a.xs.as_ptr().add(i)).reg, Packet4::load(b.xs.as_ptr().add(i)).reg);
            let my = mul(Packet4::load(a.ys.as_ptr().add(i)).reg, Packet4::load(b.ys.as_ptr().add(i)).reg);
            let mz = mul(Packet4::load(a.zs.as_ptr().add(i)).reg, Packet4::load(b.zs.as_ptr().add(i)).reg);
            let sum = F32x4 { lanes: [mx.lanes[0]+my.lanes[0]+mz.lanes[0], mx.lanes[1]+my.lanes[1]+mz.lanes[1], mx.lanes[2]+my.lanes[2]+mz.lanes[2], mx.lanes[3]+my.lanes[3]+mz.lanes[3]] };
            ptr::copy_nonoverlapping(sum.lanes.as_ptr(), out.as_mut_ptr().add(i), 4);
            i += 4;
        }
    }
    n
}

pub fn impl_lwc_sub_and_cast(world: &[DVec3], cam: DVec3) -> Vec3SoA {
    let n = world.len();
    let mut out = Vec3SoA::with_len(n);
    for i in 0..n { out.xs[i] = (world[i].x - cam.x) as f32; out.ys[i] = (world[i].y - cam.y) as f32; out.zs[i] = (world[i].z - cam.z) as f32; }
    out
}

pub fn impl_lwc_sub_and_cast_view(world: DVec3SoAView, cam: DVec3, out: Vec3SoAMutView) -> usize {
    let n = world.len().min(out.len());
    let mut i = 0usize;
    while i + 8 <= n {
        unsafe {
            let wx = [world.x[i]-cam.x, world.x[i+1]-cam.x, world.x[i+2]-cam.x, world.x[i+3]-cam.x, world.x[i+4]-cam.x, world.x[i+5]-cam.x, world.x[i+6]-cam.x, world.x[i+7]-cam.x];
            let wy = [world.y[i]-cam.y, world.y[i+1]-cam.y, world.y[i+2]-cam.y, world.y[i+3]-cam.y, world.y[i+4]-cam.y, world.y[i+5]-cam.y, world.y[i+6]-cam.y, world.y[i+7]-cam.y];
            let wz = [world.z[i]-cam.z, world.z[i+1]-cam.z, world.z[i+2]-cam.z, world.z[i+3]-cam.z, world.z[i+4]-cam.z, world.z[i+5]-cam.z, world.z[i+6]-cam.z, world.z[i+7]-cam.z];
            let rx = [wx[0] as f32, wx[1] as f32, wx[2] as f32, wx[3] as f32, wx[4] as f32, wx[5] as f32, wx[6] as f32, wx[7] as f32];
            let ry = [wy[0] as f32, wy[1] as f32, wy[2] as f32, wy[3] as f32, wy[4] as f32, wy[5] as f32, wy[6] as f32, wy[7] as f32];
            let rz = [wz[0] as f32, wz[1] as f32, wz[2] as f32, wz[3] as f32, wz[4] as f32, wz[5] as f32, wz[6] as f32, wz[7] as f32];
            ptr::copy_nonoverlapping(rx.as_ptr(), out.x.as_mut_ptr().add(i), 8);
            ptr::copy_nonoverlapping(ry.as_ptr(), out.y.as_mut_ptr().add(i), 8);
            ptr::copy_nonoverlapping(rz.as_ptr(), out.z.as_mut_ptr().add(i), 8);
        }
        i += 8;
    }
    while i < n { out.x[i] = (world.x[i]-cam.x) as f32; out.y[i] = (world.y[i]-cam.y) as f32; out.z[i] = (world.z[i]-cam.z) as f32; i += 1; }
    n
}

pub fn impl_vec3_add_view(a: Vec3SoAView, b: Vec3SoAView, out: Vec3SoAMutView) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    let mut i = 0usize;
    while i + 4 <= n {
        unsafe {
            let ax = Packet4::load(a.x.as_ptr().add(i));
            let bx = Packet4::load(b.x.as_ptr().add(i));
            ax.add(bx).store(out.x.as_mut_ptr().add(i));

            let ay = Packet4::load(a.y.as_ptr().add(i));
            let by = Packet4::load(b.y.as_ptr().add(i));
            ay.add(by).store(out.y.as_mut_ptr().add(i));

            let az = Packet4::load(a.z.as_ptr().add(i));
            let bz = Packet4::load(b.z.as_ptr().add(i));
            az.add(bz).store(out.z.as_mut_ptr().add(i));
        }
        i += 4;
    }
    while i < n { out.x[i] = a.x[i] + b.x[i]; out.y[i] = a.y[i] + b.y[i]; out.z[i] = a.z[i] + b.z[i]; i += 1; }
    n
}

pub fn impl_vec3_fma_view(a: Vec3SoAView, b: Vec3SoAView, dt: f32, out: Vec3SoAMutView) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    let dt4 = F32x4 { lanes: [dt, dt, dt, dt] };
    let mut i = 0usize;
    while i + 8 <= n {
        unsafe {
            let ax8 = Packet8::load(a.x.as_ptr().add(i));
            let bx8 = Packet8::load(b.x.as_ptr().add(i));
            let dt8 = F32x8 { lanes: [dt, dt, dt, dt, dt, dt, dt, dt] };
            let rx = fma8(bx8.reg, dt8, ax8.reg);
            let _ = store8(out.x.as_mut_ptr().add(i), rx);

            let ay8 = Packet8::load(a.y.as_ptr().add(i));
            let by8 = Packet8::load(b.y.as_ptr().add(i));
            let ry = fma8(by8.reg, dt8, ay8.reg);
            let _ = store8(out.y.as_mut_ptr().add(i), ry);

            let az8 = Packet8::load(a.z.as_ptr().add(i));
            let bz8 = Packet8::load(b.z.as_ptr().add(i));
            let rz = fma8(bz8.reg, dt8, az8.reg);
            let _ = store8(out.z.as_mut_ptr().add(i), rz);
        }
        i += 8;
    }
    while i + 4 <= n {
        unsafe {
            let ax = Packet4::load(a.x.as_ptr().add(i));
            let bx = Packet4::load(b.x.as_ptr().add(i));
            Packet4 { reg: fma(bx.reg, dt4, ax.reg) }.store(out.x.as_mut_ptr().add(i));

            let ay = Packet4::load(a.y.as_ptr().add(i));
            let by = Packet4::load(b.y.as_ptr().add(i));
            Packet4 { reg: fma(by.reg, dt4, ay.reg) }.store(out.y.as_mut_ptr().add(i));

            let az = Packet4::load(a.z.as_ptr().add(i));
            let bz = Packet4::load(b.z.as_ptr().add(i));
            Packet4 { reg: fma(bz.reg, dt4, az.reg) }.store(out.z.as_mut_ptr().add(i));
        }
        i += 4;
    }
    while i < n { out.x[i] = a.x[i] + b.x[i] * dt; out.y[i] = a.y[i] + b.y[i] * dt; out.z[i] = a.z[i] + b.z[i] * dt; i += 1; }
    n
}

pub fn impl_vec3_fma_view_unchecked(a: Vec3SoAView, b: Vec3SoAView, dt: f32, out: Vec3SoAMutView) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    let dt4 = F32x4 { lanes: [dt, dt, dt, dt] };
    let mut i = 0usize;
    unsafe {
        while i + 8 <= n {
            let bx8 = Packet8::load(b.x.as_ptr().add(i));
            let ax8 = Packet8::load(a.x.as_ptr().add(i));
            let dt8 = F32x8 { lanes: [dt, dt, dt, dt, dt, dt, dt, dt] };
            let rx = fma8(bx8.reg, dt8, ax8.reg);
            let _ = store8(out.x.as_mut_ptr().add(i), rx);

            let by8 = Packet8::load(b.y.as_ptr().add(i));
            let ay8 = Packet8::load(a.y.as_ptr().add(i));
            let ry = fma8(by8.reg, dt8, ay8.reg);
            let _ = store8(out.y.as_mut_ptr().add(i), ry);

            let bz8 = Packet8::load(b.z.as_ptr().add(i));
            let az8 = Packet8::load(a.z.as_ptr().add(i));
            let rz = fma8(bz8.reg, dt8, az8.reg);
            let _ = store8(out.z.as_mut_ptr().add(i), rz);
            i += 8;
        }
        while i + 4 <= n {
            Packet4 { reg: fma(Packet4::load(b.x.as_ptr().add(i)).reg, dt4, Packet4::load(a.x.as_ptr().add(i)).reg) }.store(out.x.as_mut_ptr().add(i));
            Packet4 { reg: fma(Packet4::load(b.y.as_ptr().add(i)).reg, dt4, Packet4::load(a.y.as_ptr().add(i)).reg) }.store(out.y.as_mut_ptr().add(i));
            Packet4 { reg: fma(Packet4::load(b.z.as_ptr().add(i)).reg, dt4, Packet4::load(a.z.as_ptr().add(i)).reg) }.store(out.z.as_mut_ptr().add(i));
            i += 4;
        }
    }
    n
}

fn sincos_scalar_precise(x: f32) -> (f32, f32) {
    let x64 = x as f64;
    let half_pi = std::f64::consts::FRAC_PI_2;
    let k = (x64 / half_pi).round() as i32;
    let r = (x64 - (k as f64) * half_pi) as f32;
    let r2 = r * r;
    let r4 = r2 * r2;
    let r6 = r4 * r2;
    let r3 = r2 * r;
    let r5 = r4 * r;
    let r7 = r6 * r;
    let sin_r = r + (-1.0/6.0)*r3 + (1.0/120.0)*r5 + (-1.0/5040.0)*r7;
    let cos_r = 1.0 + (-0.5)*r2 + (1.0/24.0)*r4 + (-1.0/720.0)*r6;
    match k & 3 {
        0 => (sin_r, cos_r),
        1 => (cos_r, -sin_r),
        2 => (-sin_r, -cos_r),
        _ => (-cos_r, sin_r),
    }
}

pub fn impl_sin_view(xs: &[f32], out: &mut [f32]) -> usize {
    let n = xs.len().min(out.len());
    for i in 0..n { out[i] = sincos_scalar_precise(xs[i]).0; }
    n
}

pub fn impl_cos_view(xs: &[f32], out: &mut [f32]) -> usize {
    let n = xs.len().min(out.len());
    for i in 0..n { out[i] = sincos_scalar_precise(xs[i]).1; }
    n
}
pub fn impl_tan_view(xs: &[f32], out: &mut [f32]) -> usize {
    let n = xs.len().min(out.len());
    for i in 0..n { let (s,c) = sincos_scalar_precise(xs[i]); out[i] = s/c; }
    n
}

pub fn impl_exp_view(xs: &[f32], out: &mut [f32]) -> usize {
    let n = xs.len().min(out.len());
    for i in 0..n { out[i] = xs[i].exp(); }
    n
}

pub fn impl_log_view(xs: &[f32], out: &mut [f32]) -> usize {
    let n = xs.len().min(out.len());
    for i in 0..n { out[i] = xs[i].ln(); }
    n
}

pub fn impl_atan2_view(ys: &[f32], xs: &[f32], out: &mut [f32]) -> usize {
    let n = ys.len().min(xs.len()).min(out.len());
    for i in 0..n { out[i] = ys[i].atan2(xs[i]); }
    n
}

pub fn impl_rect_contains(r: Rect, p: Vec2) -> bool { p.x >= r.min.x && p.y >= r.min.y && p.x <= r.max.x && p.y <= r.max.y }
pub fn impl_rect_intersects(a: Rect, b: Rect) -> bool { a.min.x <= b.max.x && a.max.x >= b.min.x && a.min.y <= b.max.y && a.max.y >= b.min.y }
pub fn impl_aabb_contains(b: AABB, p: Vec3) -> bool { p.x >= b.min.x && p.y >= b.min.y && p.z >= b.min.z && p.x <= b.max.x && p.y <= b.max.y && p.z <= b.max.z }
pub fn impl_aabb_intersects(a: AABB, b: AABB) -> bool { a.min.x <= b.max.x && a.max.x >= b.min.x && a.min.y <= b.max.y && a.max.y >= b.min.y && a.min.z <= b.max.z && a.max.z >= b.min.z }
pub fn impl_plane_distance_point(pl: Plane, p: Vec3) -> f32 { pl.n.dot(p) + pl.d }
pub fn impl_frustum_aabb_overlap(fr: Frustum, bb: AABB) -> bool {
    for i in 0..6 {
        let n = fr.planes[i].n;
        let p = Vec3 { x: if n.x >= 0.0 { bb.min.x } else { bb.max.x }, y: if n.y >= 0.0 { bb.min.y } else { bb.max.y }, z: if n.z >= 0.0 { bb.min.z } else { bb.max.z } };
        if impl_plane_distance_point(fr.planes[i], p) > 0.0 { return false; }
    }
    true
}
pub fn impl_ray_plane_intersect(ray: Ray, pl: Plane) -> Option<f32> {
    let denom = pl.n.dot(ray.dir);
    if denom.abs() < 1e-8 { return None; }
    let t = -(pl.n.dot(ray.o) + pl.d) / denom;
    if t >= 0.0 { Some(t) } else { None }
}
pub fn impl_ray_aabb_intersect(ray: Ray, bb: AABB) -> Option<f32> {
    let invx = if ray.dir.x != 0.0 { 1.0 / ray.dir.x } else { f32::INFINITY };
    let invy = if ray.dir.y != 0.0 { 1.0 / ray.dir.y } else { f32::INFINITY };
    let invz = if ray.dir.z != 0.0 { 1.0 / ray.dir.z } else { f32::INFINITY };
    let mut tmin = ((if invx >= 0.0 { bb.min.x } else { bb.max.x }) - ray.o.x) * invx;
    let mut tmax = ((if invx >= 0.0 { bb.max.x } else { bb.min.x }) - ray.o.x) * invx;
    let tymin = ((if invy >= 0.0 { bb.min.y } else { bb.max.y }) - ray.o.y) * invy;
    let tymax = ((if invy >= 0.0 { bb.max.y } else { bb.min.y }) - ray.o.y) * invy;
    if tmin > tymax || tymin > tmax { return None; }
    if tymin > tmin { tmin = tymin; }
    if tymax < tmax { tmax = tymax; }
    let tzmin = ((if invz >= 0.0 { bb.min.z } else { bb.max.z }) - ray.o.z) * invz;
    let tzmax = ((if invz >= 0.0 { bb.max.z } else { bb.min.z }) - ray.o.z) * invz;
    if tmin > tzmax || tzmin > tmax { return None; }
    if tzmin > tmin { tmin = tzmin; }
    if tzmax < tmax { tmax = tzmax; }
    if tmax < 0.0 { return None; }
    Some(if tmin >= 0.0 { tmin } else { tmax })
}
