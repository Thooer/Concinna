mod types;
pub use types::*;
mod views;
pub use views::*;
pub mod packet;
pub mod bit;
pub use bit::*;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Ops.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
#[path = "../Impl/Generic/Ops.rs"]
mod backend;

pub fn vec4_add(a: Vec4, b: Vec4) -> Vec4 { backend::impl_vec4_add(a, b) }
pub fn vec4_add_batch(a: &[[f32;4]], b: &[[f32;4]], out: &mut [[f32;4]]) -> usize { backend::impl_vec4_add_batch(a, b, out) }
pub fn mat4_mul(a: Mat4, b: Mat4) -> Mat4 { backend::impl_mat4_mul(a, b) }

pub fn vec4_add_soa(a: &Vec4SoA, b: &Vec4SoA, out: &mut Vec4SoA) -> usize { backend::impl_vec4_add_soa(a, b, out) }
pub fn vec3_dot_soa(a: &Vec3SoA, b: &Vec3SoA, out: &mut [f32]) -> usize { backend::impl_vec3_dot_soa(a, b, out) }
pub fn lwc_sub_and_cast(world: &[DVec3], cam: DVec3) -> Vec3SoA { backend::impl_lwc_sub_and_cast(world, cam) }

pub fn vec3_add_view(a: Vec3SoAView, b: Vec3SoAView, out: Vec3SoAMutView) -> usize { backend::impl_vec3_add_view(a, b, out) }
pub fn vec3_fma_view(a: Vec3SoAView, b: Vec3SoAView, dt: f32, out: Vec3SoAMutView) -> usize { backend::impl_vec3_fma_view(a, b, dt, out) }

pub fn vec4_add_soa_unchecked(a: &Vec4SoA, b: &Vec4SoA, out: &mut Vec4SoA) -> usize { backend::impl_vec4_add_soa_unchecked(a, b, out) }
pub fn vec3_dot_soa_unchecked(a: &Vec3SoA, b: &Vec3SoA, out: &mut [f32]) -> usize { backend::impl_vec3_dot_soa_unchecked(a, b, out) }
pub fn vec3_fma_view_unchecked(a: Vec3SoAView, b: Vec3SoAView, dt: f32, out: Vec3SoAMutView) -> usize { backend::impl_vec3_fma_view_unchecked(a, b, dt, out) }

pub fn lwc_sub_and_cast_view(world: DVec3SoAView, cam: DVec3, out: Vec3SoAMutView) -> usize { backend::impl_lwc_sub_and_cast_view(world, cam, out) }
pub fn sin_view(xs: &[f32], out: &mut [f32]) -> usize { backend::impl_sin_view(xs, out) }
pub fn cos_view(xs: &[f32], out: &mut [f32]) -> usize { backend::impl_cos_view(xs, out) }
pub fn tan_view(xs: &[f32], out: &mut [f32]) -> usize { backend::impl_tan_view(xs, out) }
pub fn exp_view(xs: &[f32], out: &mut [f32]) -> usize { backend::impl_exp_view(xs, out) }
pub fn log_view(xs: &[f32], out: &mut [f32]) -> usize { backend::impl_log_view(xs, out) }
pub fn atan2_view(ys: &[f32], xs: &[f32], out: &mut [f32]) -> usize { backend::impl_atan2_view(ys, xs, out) }

pub fn rect_contains(rect: Rect, p: Vec2) -> bool { backend::impl_rect_contains(rect, p) }
pub fn rect_intersects(a: Rect, b: Rect) -> bool { backend::impl_rect_intersects(a, b) }
pub fn aabb_contains(aabb: AABB, p: Vec3) -> bool { backend::impl_aabb_contains(aabb, p) }
pub fn aabb_intersects(a: AABB, b: AABB) -> bool { backend::impl_aabb_intersects(a, b) }
pub fn plane_distance_point(pl: Plane, p: Vec3) -> f32 { backend::impl_plane_distance_point(pl, p) }
pub fn frustum_aabb_overlap(fr: Frustum, aabb: AABB) -> bool { backend::impl_frustum_aabb_overlap(fr, aabb) }
pub fn ray_aabb_intersect(ray: Ray, aabb: AABB) -> Option<f32> { backend::impl_ray_aabb_intersect(ray, aabb) }
pub fn ray_plane_intersect(ray: Ray, pl: Plane) -> Option<f32> { backend::impl_ray_plane_intersect(ray, pl) }
