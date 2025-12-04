use cap_algorithms::*;
use cap_containers::*;
use cap_memory::*;
use prm_time::*;

fn lcg(seed: &mut u64) -> u64 { *seed = seed.wrapping_mul(6364136223846793005).wrapping_add(1); *seed }

fn main() {
    let mut frame = FrameAllocatorResource::new(64<<20);
    let a = Allocator::new(&mut frame);
    let mut xs = Vector::<u64>::with_capacity(a, 1<<20).unwrap();
    let mut seed = 0xdeadbeefcafebabeu64;
    for _ in 0..(1<<20) { let v = lcg(&mut seed); xs.push(v).unwrap(); }

    let mut arr_qs = Vector::<u64>::with_capacity(a, xs.len()).unwrap();
    let mut arr_rx = Vector::<u64>::with_capacity(a, xs.len()).unwrap();
    for i in 0..xs.len() { arr_qs.push(xs.get(i).unwrap()).unwrap(); arr_rx.push(xs.get(i).unwrap()).unwrap(); }

    let t0 = now();
    quick_sort_u64(arr_qs.as_mut_slice());
    let t1 = now();
    let dt_qs = to_milliseconds(delta(t0, t1));

    radix_sort_u64(a, arr_rx.as_mut_slice()).unwrap();
    let t2 = now();
    let dt_rx = to_milliseconds(delta(t1, t2));

    println!("qs_ms {} rx_ms {} first {} last {}", dt_qs, dt_rx, arr_qs.get(0).unwrap(), arr_qs.get(arr_qs.len()-1).unwrap());
}
