use cap_math::*;

fn main() {
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
    
    // 打印结果
    println!("Matrix 1:");
    for i in 0..4 {
        println!("{:?}", m1.rows[i]);
    }
    
    println!("\nMatrix 2:");
    for i in 0..4 {
        println!("{:?}", m2.rows[i]);
    }
    
    println!("\nResult:");
    for i in 0..4 {
        println!("{:?}", result.rows[i]);
    }
    
    // 验证结果是否正确（简单的验证，实际应该有更完整的测试）
    let expected = Mat4::from_trs(
        Vec3::new(6.0, 9.0, 12.0),
        Quat::identity(),
        Vec3::new(1.0, 1.0, 1.0)
    );
    
    println!("\nExpected:");
    for i in 0..4 {
        println!("{:?}", expected.rows[i]);
    }
    
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
    
    println!("\nResult is correct: {}", all_close);
    
    // 运行性能测试
    let n = 1_000_000;
    let start = std::time::Instant::now();
    
    for _ in 0..n {
        let _ = m1.mul(m2);
    }
    
    let duration = start.elapsed();
    println!("\nPerformance: {} matrix multiplications in {:?} ({:.2} million/s)",
             n, duration, n as f64 / duration.as_secs_f64() / 1_000_000.0);
}