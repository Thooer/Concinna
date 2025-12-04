use cap_random::*;

fn main() {
    let mut r = Random::with_seed(123456789);
    let a = r.next_u32();
    let b = r.uniform_f32(0.0, 1.0);
    let c = r.normal_f32(0.0, 1.0);
    println!("rand {} {} {}", a, b, c);
}

