use cap_math::*;
use std::time::Instant;

fn lcg(seed: &mut u64) -> f32 {
    *seed = seed.wrapping_mul(6364136223846793005).wrapping_add(1);
    let v = ((*seed >> 32) as u32) as f32;
    v / (u32::MAX as f32)
}

fn test_precision() {
    let mut s = 123456789u64;
    let mut max_err = 0.0f32;
    for _ in 0..100000 {
        let v = Vec3::new(lcg(&mut s)*2.0-1.0, lcg(&mut s)*2.0-1.0, lcg(&mut s)*2.0-1.0);
        let n = v.normalize();
        let e = (n.length() - 1.0).abs();
        if e > max_err { max_err = e; }
    }
    let q = Quat::from_axis_angle(Vec3::new(0.3, 0.7, 0.1), 1.234);
    let m = Mat4::from_trs(Vec3::new(1.0,2.0,3.0), q, Vec3::new(2.0,2.0,2.0));
    let inv = m.inverse_affine();
    let id = m.mul(inv);
    let mut ortho_err = 0.0f32;
    for i in 0..4 { for j in 0..4 { let t = if i==j { 1.0 } else { 0.0 }; let d = (id.rows[i][j]-t).abs(); if d>ortho_err { ortho_err=d; } } }
    println!("precision normalize_max_err={:.6} mat_affine_inv_err={:.6}", max_err, ortho_err);
}

fn test_matmul() {
    println!("\nTesting matrix multiplication...");
    
    // 创建两个简单的矩阵
    let m1 = Mat4::from_trs(
        Vec3::new(1.0, 2.0, 3.0),
        Quat::identity(),
        Vec3::new(2.0, 2.0, 2.0)
    );
    
    let m2 = Mat4::from_trs(
        Vec3::new(4.0, 5.0, 6.0),
        Quat::identity(),
        Vec3::new(0.5, 0.5, 0.5)
    );
    
    // 使用优化后的矩阵乘法计算结果
    let result = m1.mul(m2);
    
    // 验证结果是否正确
    let expected = Mat4::from_trs(
        Vec3::new(6.0, 9.0, 12.0),
        Quat::identity(),
        Vec3::new(1.0, 1.0, 1.0)
    );
    
    // 检查结果是否接近预期
    let mut all_close = true;
    for i in 0..4 {
        for j in 0..4 {
            if (result.rows[i][j] - expected.rows[i][j]).abs() > 1e-5 {
                all_close = false;
                break;
            }
        }
    }
    
    println!("Matrix multiplication result is correct: {}", all_close);
    
    // 运行性能测试
    let n = 1_000_000;
    let start = Instant::now();
    
    for _ in 0..n {
        let _ = m1.mul(m2);
    }
    
    let duration = start.elapsed();
    println!("Performance: {} matrix multiplications in {:?} ({:.2} million/s)",
             n, duration, n as f64 / duration.as_secs_f64() / 1_000_000.0);
}

fn test_performance() {
    let n = 8_000_000usize;
    let mut a = vec![[0.0f32;4]; n];
    let mut b = vec![[0.0f32;4]; n];
    let mut out = vec![[0.0f32;4]; n];
    let mut s = 987654321u64;
    for i in 0..n { a[i] = [lcg(&mut s), lcg(&mut s), lcg(&mut s), lcg(&mut s)]; b[i] = [lcg(&mut s), lcg(&mut s), lcg(&mut s), lcg(&mut s)]; }
    let target = 1.0f64;
    let mut loops = 0usize;
    let t0 = Instant::now();
    while t0.elapsed().as_secs_f64() < target { for i in 0..n { let va = Vec4::new(a[i][0], a[i][1], a[i][2], a[i][3]); let vb = Vec4::new(b[i][0], b[i][1], b[i][2], b[i][3]); let vr = vec4_add(va, vb); out[i] = [vr.x, vr.y, vr.z, vr.w]; } loops+=1; }
    let d_scalar = t0.elapsed().as_secs_f64();
    let mut loops2 = 0usize;
    let t1 = Instant::now();
    while t1.elapsed().as_secs_f64() < target { let _ = vec4_add_batch(&a, &b, &mut out); loops2+=1; }
    let d_simd = t1.elapsed().as_secs_f64();
    println!("perf vec4_add_aos scalar={:.3}s simd={:.3}s loops_s={} loops_v={}", d_scalar, d_simd, loops, loops2);

    let mut aos = Vec::with_capacity(n);
    for i in 0..n { aos.push(Vec4::new(a[i][0], a[i][1], a[i][2], a[i][3])); }
    let mut bos = Vec::with_capacity(n);
    for i in 0..n { bos.push(Vec4::new(b[i][0], b[i][1], b[i][2], b[i][3])); }
    let soa_a = Vec4SoA::from_aos(&aos);
    let soa_b = Vec4SoA::from_aos(&bos);
    let mut soa_out = Vec4SoA::with_len(n);
    let mut loops3 = 0usize;
    let t2 = Instant::now();
    while t2.elapsed().as_secs_f64() < target { let _ = vec4_add_soa(&soa_a, &soa_b, &mut soa_out); loops3+=1; }
    let d_soa = t2.elapsed().as_secs_f64();
    let mut loops4 = 0usize;
    let t3 = Instant::now();
    while t3.elapsed().as_secs_f64() < target { let _ = vec4_add_soa_unchecked(&soa_a, &soa_b, &mut soa_out); loops4+=1; }
    let d_soa_u = t3.elapsed().as_secs_f64();
    println!("perf vec4_add_soa simd={:.3}s unchecked={:.3}s loops_s={} loops_u={}", d_soa, d_soa_u, loops3, loops4);
}

fn main() {
    test_precision();
    test_matmul();
    test_performance();

    let world = vec![DVec3::new(1e6, 2e6, -3e6), DVec3::new(1.0, 2.0, 3.0)];
    let cam = DVec3::new(1e6 + 10.0, 2e6 - 5.0, -3e6 + 2.5);
    let local = lwc_sub_and_cast(&world, cam);
    println!("lwc first=({:.3},{:.3},{:.3})", local.xs[0], local.ys[0], local.zs[0]);

    // View kernels demo
    let target = 1.0f64;
    let n = 2_000_000usize;
    let mut px = vec![0.0f32; n]; let mut py = vec![0.0f32; n]; let mut pz = vec![0.0f32; n];
    let mut vx = vec![0.0f32; n]; let mut vy = vec![0.0f32; n]; let mut vz = vec![0.0f32; n];
    let mut s = 55555555u64; let dt = 0.016f32;
    for i in 0..n {
        let r = |seed: &mut u64| { *seed = seed.wrapping_mul(6364136223846793005).wrapping_add(1); ((*seed >> 32) as u32) as f32 / (u32::MAX as f32) };
        px[i] = r(&mut s); py[i] = r(&mut s); pz[i] = r(&mut s);
        vx[i] = r(&mut s); vy[i] = r(&mut s); vz[i] = r(&mut s);
    }
    let view_p = Vec3SoAView { x: &px, y: &py, z: &pz };
    let view_v = Vec3SoAView { x: &vx, y: &vy, z: &vz };
    let mut outx = vec![0.0f32; n]; let mut outy = vec![0.0f32; n]; let mut outz = vec![0.0f32; n];
    let mut loops5 = 0usize;
    let t0 = Instant::now();
    while t0.elapsed().as_secs_f64() < target { let _ = vec3_fma_view(view_p, view_v, dt, Vec3SoAMutView { x: &mut outx, y: &mut outy, z: &mut outz }); loops5+=1; }
    let d = t0.elapsed().as_secs_f64();
    let mut loops6 = 0usize;
    let t1 = Instant::now();
    while t1.elapsed().as_secs_f64() < target { let _ = vec3_fma_view_unchecked(view_p, view_v, dt, Vec3SoAMutView { x: &mut outx, y: &mut outy, z: &mut outz }); loops6+=1; }
    let d_u = t1.elapsed().as_secs_f64();
    println!("perf vec3_fma_view simd={:.3}s unchecked={:.3}s loops_s={} loops_u={}", d, d_u, loops5, loops6);

    let m = 16_000_000usize;
    let mut xs = vec![0.0f32; m];
    let mut sin_out = vec![0.0f32; m];
    let mut cos_out = vec![0.0f32; m];
    let mut s2 = 7777777u64;
    for i in 0..m { xs[i] = lcg(&mut s2)*2.0 - 1.0; }
    let mut loops7 = 0usize;
    let t_s = Instant::now();
    while t_s.elapsed().as_secs_f64() < target { let _ = sin_view(&xs, &mut sin_out); loops7+=1; }
    let ds = t_s.elapsed().as_secs_f64();
    let mut loops8 = 0usize;
    let t_c = Instant::now();
    while t_c.elapsed().as_secs_f64() < target { let _ = cos_view(&xs, &mut cos_out); loops8+=1; }
    let dc = t_c.elapsed().as_secs_f64();
    let mut max_err_s = 0.0f32; let mut max_err_c = 0.0f32;
    for i in 0..m { let es = (sin_out[i] - xs[i].sin()).abs(); if es > max_err_s { max_err_s = es; } let ec = (cos_out[i] - xs[i].cos()).abs(); if ec > max_err_c { max_err_c = ec; } }
    let mut loops9 = 0usize;
    let t_t = Instant::now();
    let mut tan_out = vec![0.0f32; m];
    while t_t.elapsed().as_secs_f64() < target { let _ = tan_view(&xs, &mut tan_out); loops9+=1; }
    let dtan = t_t.elapsed().as_secs_f64();
    let mut loops10 = 0usize;
    let t_e = Instant::now();
    let mut exp_out = vec![0.0f32; m];
    while t_e.elapsed().as_secs_f64() < target { let _ = exp_view(&xs, &mut exp_out); loops10+=1; }
    let dexp = t_e.elapsed().as_secs_f64();
    let mut loops11 = 0usize;
    let t_l = Instant::now();
    let mut log_out = vec![0.0f32; m];
    while t_l.elapsed().as_secs_f64() < target { let _ = log_view(&xs, &mut log_out); loops11+=1; }
    let dlog = t_l.elapsed().as_secs_f64();
    let mut loops12 = 0usize;
    let t_a = Instant::now();
    let mut atan2_out = vec![0.0f32; m];
    while t_a.elapsed().as_secs_f64() < target { let _ = atan2_view(&sin_out, &cos_out, &mut atan2_out); loops12+=1; }
    let datan2 = t_a.elapsed().as_secs_f64();
    println!("perf sin={:.3}s cos={:.3}s tan={:.3}s exp={:.3}s log={:.3}s atan2={:.3}s loops_s={} loops_c={} loops_t={} loops_e={} loops_l={} loops_a={} err_s={:.6} err_c={:.6}", ds, dc, dtan, dexp, dlog, datan2, loops7, loops8, loops9, loops10, loops11, loops12, max_err_s, max_err_c);

    let fr = Frustum::new([
        Plane::from_point_normal(Vec3::new(-1.0,0.0,0.0), Vec3::new(-1.0,0.0,0.0)),
        Plane::from_point_normal(Vec3::new(1.0,0.0,0.0), Vec3::new(1.0,0.0,0.0)),
        Plane::from_point_normal(Vec3::new(0.0,-1.0,0.0), Vec3::new(0.0,-1.0,0.0)),
        Plane::from_point_normal(Vec3::new(0.0,1.0,0.0), Vec3::new(0.0,1.0,0.0)),
        Plane::from_point_normal(Vec3::new(0.0,0.0,-1.0), Vec3::new(0.0,0.0,-1.0)),
        Plane::from_point_normal(Vec3::new(0.0,0.0,1.0), Vec3::new(0.0,0.0,1.0)),
    ]);
    let bb_in = AABB::from_center_extent(Vec3::new(0.0,0.0,0.0), Vec3::new(0.5,0.5,0.5));
    let bb_out = AABB::from_center_extent(Vec3::new(2.0,2.0,2.0), Vec3::new(0.5,0.5,0.5));
    let c_in = frustum_aabb_overlap(fr, bb_in);
    let c_out = frustum_aabb_overlap(fr, bb_out);
    let pl = Plane::from_point_normal(Vec3::new(0.0,0.0,1.0), Vec3::new(0.0,0.0,1.0));
    let d0 = plane_distance_point(pl, Vec3::new(0.0,0.0,0.0));
    let ray = Ray::new(Vec3::new(0.0,0.0,-2.0), Vec3::new(0.0,0.0,1.0));
    let hit = ray_aabb_intersect(ray, AABB::from_center_extent(Vec3::new(0.0,0.0,0.0), Vec3::new(1.0,1.0,1.0)));
    println!("geom frustum_in={} frustum_out={} plane_d0={:.3} ray_box_t={:?}", c_in, c_out, d0, hit);
}
