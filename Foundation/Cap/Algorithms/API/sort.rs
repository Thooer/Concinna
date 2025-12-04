pub fn quick_sort_u64(xs: &mut [u64]) -> usize {
    if xs.len() <= 1 { return xs.len(); }
    let mut stack: [usize; 128] = [0; 128];
    let mut sp = 0usize;
    stack[sp] = 0; sp += 1; stack[sp] = xs.len()-1; sp += 1;
    while sp > 0 {
        let hi = stack[sp-1]; sp -= 1;
        let lo = stack[sp-1]; sp -= 1;
        if hi <= lo { continue; }
        let p = partition_u64(xs, lo, hi);
        if p > 0 { if p-1 > lo { stack[sp] = lo; sp += 1; stack[sp] = p-1; sp += 1; } }
        if p+1 < hi { stack[sp] = p+1; sp += 1; stack[sp] = hi; sp += 1; }
    }
    xs.len()
}

fn partition_u64(xs: &mut [u64], lo: usize, hi: usize) -> usize {
    let pivot = xs[hi];
    let mut i = lo;
    for j in lo..hi {
        if xs[j] <= pivot { xs.swap(i, j); i += 1; }
    }
    xs.swap(i, hi);
    i
}

pub fn quick_sort_f32(xs: &mut [f32]) -> usize {
    if xs.len() <= 1 { return xs.len(); }
    let mut stack: [usize; 128] = [0; 128];
    let mut sp = 0usize;
    stack[sp] = 0; sp += 1; stack[sp] = xs.len()-1; sp += 1;
    while sp > 0 {
        let hi = stack[sp-1]; sp -= 1;
        let lo = stack[sp-1]; sp -= 1;
        if hi <= lo { continue; }
        let p = partition_f32(xs, lo, hi);
        if p > 0 { if p-1 > lo { stack[sp] = lo; sp += 1; stack[sp] = p-1; sp += 1; } }
        if p+1 < hi { stack[sp] = p+1; sp += 1; stack[sp] = hi; sp += 1; }
    }
    xs.len()
}

fn partition_f32(xs: &mut [f32], lo: usize, hi: usize) -> usize {
    let pivot = xs[hi];
    let mut i = lo;
    for j in lo..hi {
        if xs[j] <= pivot { xs.swap(i, j); i += 1; }
    }
    xs.swap(i, hi);
    i
}

pub fn binary_search_u64(xs: &[u64], key: u64) -> Option<usize> {
    let mut lo = 0usize; let mut hi = xs.len();
    while lo < hi { let mid = (lo + hi) >> 1; let v = xs[mid]; if v < key { lo = mid + 1; } else { hi = mid; } }
    if lo < xs.len() && xs[lo] == key { Some(lo) } else { None }
}

