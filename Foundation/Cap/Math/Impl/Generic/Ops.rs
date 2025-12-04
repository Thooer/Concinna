use crate::*;
use crate::{Vec3SoAView, Vec3SoAMutView};
use std::ptr;

pub fn impl_vec4_add(a: Vec4, b: Vec4) -> Vec4 { Vec4 { x: a.x + b.x, y: a.y + b.y, z: a.z + b.z, w: a.w + b.w } }

pub fn impl_vec4_add_batch(a: &[[f32;4]], b: &[[f32;4]], out: &mut [[f32;4]]) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    for i in 0..n {
        out[i][0] = a[i][0] + b[i][0];
        out[i][1] = a[i][1] + b[i][1];
        out[i][2] = a[i][2] + b[i][2];
        out[i][3] = a[i][3] + b[i][3];
    }
    n
}

pub fn impl_mat4_mul(a: Mat4, b: Mat4) -> Mat4 {
    let bt = b.transpose();
    let mut r = [[0.0;4];4];
    for i in 0..4 {
        for j in 0..4 {
            r[i][j] = a.rows[i][0]*bt.rows[j][0] + a.rows[i][1]*bt.rows[j][1] + a.rows[i][2]*bt.rows[j][2] + a.rows[i][3]*bt.rows[j][3];
        }
    }
    Mat4 { rows: r }
}

pub fn impl_vec4_add_soa(a: &Vec4SoA, b: &Vec4SoA, out: &mut Vec4SoA) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    let mut i = 0usize;
    while i + 4 <= n {
        unsafe {
            let ax = [a.xs[i], a.xs[i+1], a.xs[i+2], a.xs[i+3]];
            let bx = [b.xs[i], b.xs[i+1], b.xs[i+2], b.xs[i+3]];
            let rx = [ax[0]+bx[0], ax[1]+bx[1], ax[2]+bx[2], ax[3]+bx[3]];
            ptr::copy_nonoverlapping(rx.as_ptr(), out.xs.as_mut_ptr().add(i), 4);

            let ay = [a.ys[i], a.ys[i+1], a.ys[i+2], a.ys[i+3]];
            let by = [b.ys[i], b.ys[i+1], b.ys[i+2], b.ys[i+3]];
            let ry = [ay[0]+by[0], ay[1]+by[1], ay[2]+by[2], ay[3]+by[3]];
            ptr::copy_nonoverlapping(ry.as_ptr(), out.ys.as_mut_ptr().add(i), 4);

            let az = [a.zs[i], a.zs[i+1], a.zs[i+2], a.zs[i+3]];
            let bz = [b.zs[i], b.zs[i+1], b.zs[i+2], b.zs[i+3]];
            let rz = [az[0]+bz[0], az[1]+bz[1], az[2]+bz[2], az[3]+bz[3]];
            ptr::copy_nonoverlapping(rz.as_ptr(), out.zs.as_mut_ptr().add(i), 4);

            let aw = [a.ws[i], a.ws[i+1], a.ws[i+2], a.ws[i+3]];
            let bw = [b.ws[i], b.ws[i+1], b.ws[i+2], b.ws[i+3]];
            let rw = [aw[0]+bw[0], aw[1]+bw[1], aw[2]+bw[2], aw[3]+bw[3]];
            ptr::copy_nonoverlapping(rw.as_ptr(), out.ws.as_mut_ptr().add(i), 4);
        }
        i += 4;
    }
    while i < n { out.xs[i] = a.xs[i] + b.xs[i]; out.ys[i] = a.ys[i] + b.ys[i]; out.zs[i] = a.zs[i] + b.zs[i]; out.ws[i] = a.ws[i] + b.ws[i]; i += 1; }
    n
}

pub fn impl_vec3_dot_soa(a: &Vec3SoA, b: &Vec3SoA, out: &mut [f32]) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    let mut i = 0usize;
    while i + 4 <= n {
        unsafe {
            let rx0 = a.xs[i]*b.xs[i] + a.ys[i]*b.ys[i] + a.zs[i]*b.zs[i];
            let rx1 = a.xs[i+1]*b.xs[i+1] + a.ys[i+1]*b.ys[i+1] + a.zs[i+1]*b.zs[i+1];
            let rx2 = a.xs[i+2]*b.xs[i+2] + a.ys[i+2]*b.ys[i+2] + a.zs[i+2]*b.zs[i+2];
            let rx3 = a.xs[i+3]*b.xs[i+3] + a.ys[i+3]*b.ys[i+3] + a.zs[i+3]*b.zs[i+3];
            let vec = [rx0, rx1, rx2, rx3];
            ptr::copy_nonoverlapping(vec.as_ptr(), out.as_mut_ptr().add(i), 4);
        }
        i += 4;
    }
    while i < n { out[i] = a.xs[i]*b.xs[i] + a.ys[i]*b.ys[i] + a.zs[i]*b.zs[i]; i += 1; }
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
    for i in 0..n {
        out.x[i] = (world.x[i]-cam.x) as f32;
        out.y[i] = (world.y[i]-cam.y) as f32;
        out.z[i] = (world.z[i]-cam.z) as f32;
    }
    n
}

pub fn impl_vec3_add_view(a: Vec3SoAView, b: Vec3SoAView, out: Vec3SoAMutView) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    for i in 0..n {
        out.x[i] = a.x[i] + b.x[i];
        out.y[i] = a.y[i] + b.y[i];
        out.z[i] = a.z[i] + b.z[i];
    }
    n
}

pub fn impl_vec3_fma_view(a: Vec3SoAView, b: Vec3SoAView, dt: f32, out: Vec3SoAMutView) -> usize {
    let n = a.len().min(b.len()).min(out.len());
    for i in 0..n {
        out.x[i] = a.x[i] + b.x[i] * dt;
        out.y[i] = a.y[i] + b.y[i] * dt;
        out.z[i] = a.z[i] + b.z[i] * dt;
    }
    n
}

pub fn impl_sin_view(xs: &[f32], out: &mut [f32]) -> usize {
    let n = xs.len().min(out.len());
    for i in 0..n { out[i] = xs[i].sin(); }
    n
}

pub fn impl_cos_view(xs: &[f32], out: &mut [f32]) -> usize {
    let n = xs.len().min(out.len());
    for i in 0..n { out[i] = xs[i].cos(); }
    n
}

pub fn impl_tan_view(xs: &[f32], out: &mut [f32]) -> usize {
    let n = xs.len().min(out.len());
    for i in 0..n { out[i] = xs[i].tan(); }
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
