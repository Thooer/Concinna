use cap_containers::Vector;

pub fn find_if_slice<T: Copy, F: Fn(T) -> bool>(xs: &[T], f: F) -> Option<usize> {
    for i in 0..xs.len() { if f(xs[i]) { return Some(i); } }
    None
}

pub fn find_if_vector<'a, T: Copy, F: Fn(T) -> bool>(xs: &Vector<'a, T>, f: F) -> Option<usize> {
    find_if_slice(xs.as_slice(), f)
}

