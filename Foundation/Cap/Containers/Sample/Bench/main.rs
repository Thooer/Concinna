use std::time::Instant;
use cap_memory::*;
use cap_containers::*;
use lang_derive::SoA;
use prm_simd::*;
use std::hint::black_box;


#[derive(Clone, Copy, SoA)]
struct Vec3 { x: f32, y: f32, z: f32 }

#[inline(never)]
fn aos_scalar_step(v: Vec3, c: Vec3) -> f32 {
    unsafe {
        let px = &v.x as *const f32;
        let py = &v.y as *const f32;
        let pz = &v.z as *const f32;
        let x = std::ptr::read_volatile(px);
        let y = std::ptr::read_volatile(py);
        let z = std::ptr::read_volatile(pz);
        x * c.x + y * c.y + z * c.z
    }
}

fn main() {
    let n = 1_000_000usize;
    let mut frame = FrameAllocatorResource::new(128 << 20);
    let a = Allocator::new(&mut frame);

    let align = 32;
    let mut soa = Vec3SoA::<UseVector>::with_capacity_aligned(a, n, align).unwrap();

    for i in 0..n {
        let f = (i as f32) * 0.001;

        soa.push(Vec3 { x: f, y: f + 1.0, z: f + 2.0 }).unwrap();
    }

    let mut aos: Vec<Vec3> = Vec::with_capacity(n);
    for i in 0..n {
        let f = (i as f32) * 0.001;
        aos.push(Vec3 { x: f, y: f + 1.0, z: f + 2.0 });
    }

    let c = Vec3 { x: 0.7, y: 1.3, z: -0.2 };

    {
        let mut d0 = 0.0;
        let mut d1 = 0.0;
        let mut d2 = 0.0;
        for i in 0..n { d0 += soa.x.get(i).unwrap(); }
        for i in 0..n { d1 += soa.y.get(i).unwrap(); }
        for i in 0..n { d2 += soa.z.get(i).unwrap(); }
        black_box(d0 + d1 + d2);
    }

    let t0 = Instant::now();

    let cx = c.x; let cy = c.y; let cz = c.z;

    // 1. 增加累加器数量到 8 个，充分掩盖 FMA 延迟
    let mut acc0 = F32x8 { lanes: [0.0; 8] };
    let mut acc1 = F32x8 { lanes: [0.0; 8] };
    let mut acc2 = F32x8 { lanes: [0.0; 8] };
    let mut acc3 = F32x8 { lanes: [0.0; 8] };
    let mut acc4 = F32x8 { lanes: [0.0; 8] };
    let mut acc5 = F32x8 { lanes: [0.0; 8] };
    let mut acc6 = F32x8 { lanes: [0.0; 8] };
    let mut acc7 = F32x8 { lanes: [0.0; 8] };

    let ptr_x = soa.x.as_ptr();
    let ptr_y = soa.y.as_ptr();
    let ptr_z = soa.z.as_ptr();

    let mut i = 0usize;
    // 每次处理 8 * 8 = 64 个元素
    while i + 64 <= n {
        unsafe {
            // 3. 充分的循环展开
            let d0 = dot3_fma8_aligned(ptr_x.add(i),      ptr_y.add(i),      ptr_z.add(i),      cx, cy, cz);
            let d1 = dot3_fma8_aligned(ptr_x.add(i + 8),  ptr_y.add(i + 8),  ptr_z.add(i + 8),  cx, cy, cz);
            let d2 = dot3_fma8_aligned(ptr_x.add(i + 16), ptr_y.add(i + 16), ptr_z.add(i + 16), cx, cy, cz);
            let d3 = dot3_fma8_aligned(ptr_x.add(i + 24), ptr_y.add(i + 24), ptr_z.add(i + 24), cx, cy, cz);
            let d4 = dot3_fma8_aligned(ptr_x.add(i + 32), ptr_y.add(i + 32), ptr_z.add(i + 32), cx, cy, cz);
            let d5 = dot3_fma8_aligned(ptr_x.add(i + 40), ptr_y.add(i + 40), ptr_z.add(i + 40), cx, cy, cz);
            let d6 = dot3_fma8_aligned(ptr_x.add(i + 48), ptr_y.add(i + 48), ptr_z.add(i + 48), cx, cy, cz);
            let d7 = dot3_fma8_aligned(ptr_x.add(i + 56), ptr_y.add(i + 56), ptr_z.add(i + 56), cx, cy, cz);

            // 累加到独立的寄存器，打破依赖链
            acc0 = add8(acc0, d0);
            acc1 = add8(acc1, d1);
            acc2 = add8(acc2, d2);
            acc3 = add8(acc3, d3);
            acc4 = add8(acc4, d4);
            acc5 = add8(acc5, d5);
            acc6 = add8(acc6, d6);
            acc7 = add8(acc7, d7);
        }
        i += 64;
    }

    // 4. 聚合结果
    let sum_a = add8(add8(acc0, acc1), add8(acc2, acc3));
    let sum_b = add8(add8(acc4, acc5), add8(acc6, acc7));
    let total_vec = add8(sum_a, sum_b);
    
    let mut acc_soa = hsum8(total_vec);

    // 处理尾部
    for j in i..n {
        let x = soa.x.get(j).unwrap();
        let y = soa.y.get(j).unwrap();
        let z = soa.z.get(j).unwrap();
        acc_soa += x * c.x + y * c.y + z * c.z;
    }

    let dt_soa = t0.elapsed().as_micros();

    let t1 = Instant::now();
    let mut acc_aos: f32 = 0.0;
    for j in 0..n {
        // 正常的指针读取，允许编译器优化
        let v = unsafe { *aos.as_ptr().add(j) }; 
        // 纯计算
        acc_aos += v.x * c.x + v.y * c.y + v.z * c.z;
    }
    black_box(acc_aos);
    let dt_aos = t1.elapsed().as_micros();

    println!("N = {}", n);
    println!("SoA (AVX2 FMA): {:.3} ms (Result: {:.2})", dt_soa as f64 / 1000.0, acc_soa);
    println!("AoS (Scalar)  : {:.3} ms (Result: {:.2})", dt_aos as f64 / 1000.0, acc_aos);
    if dt_soa > 0 { println!("Speedup: {:.2}x", dt_aos as f64 / dt_soa as f64); }
}
