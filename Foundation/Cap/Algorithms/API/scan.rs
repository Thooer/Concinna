pub fn prefix_sum_u64_inclusive(xs: &mut [u64]) -> usize {
    let mut acc = 0u64;
    for i in 0..xs.len() { acc = acc.wrapping_add(xs[i]); xs[i] = acc; }
    xs.len()
}

pub fn prefix_sum_u64_exclusive(xs: &mut [u64]) -> usize {
    let mut acc = 0u64;
    for i in 0..xs.len() { let v = xs[i]; xs[i] = acc; acc = acc.wrapping_add(v); }
    xs.len()
}

pub fn prefix_sum_f32_inclusive(xs: &mut [f32]) -> usize {
    let mut acc = 0.0f32;
    for i in 0..xs.len() { acc += xs[i]; xs[i] = acc; }
    xs.len()
}

