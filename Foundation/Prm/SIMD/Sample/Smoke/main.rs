use prm_simd::*;

fn main() {
    let a = F32x4 { lanes: [1.0, 2.0, 3.0, 4.0] };
    let b = F32x4 { lanes: [5.0, 6.0, 7.0, 8.0] };
    let c = F32x4 { lanes: [0.5, 0.5, 0.5, 0.5] };
    let c2 = add(a, b);
    let d = mul(a, b);
    let s = dot(a, b);
    println!("add: {:.1} {:.1} {:.1} {:.1}", c2.lanes[0], c2.lanes[1], c2.lanes[2], c2.lanes[3]);
    println!("mul: {:.1} {:.1} {:.1} {:.1}", d.lanes[0], d.lanes[1], d.lanes[2], d.lanes[3]);
    println!("dot: {:.1}", s);
    let f = fma(a, b, c);
    println!("fma: {:.1} {:.1} {:.1} {:.1}", f.lanes[0], f.lanes[1], f.lanes[2], f.lanes[3]);
    let mn = min(a, b);
    let mx = max(a, b);
    println!("min: {:.1} {:.1} {:.1} {:.1}", mn.lanes[0], mn.lanes[1], mn.lanes[2], mn.lanes[3]);
    println!("max: {:.1} {:.1} {:.1} {:.1}", mx.lanes[0], mx.lanes[1], mx.lanes[2], mx.lanes[3]);
    let m = cmp_lt(a, b);
    let sel = select(m, a, b);
    println!("sel: {:.1} {:.1} {:.1} {:.1}", sel.lanes[0], sel.lanes[1], sel.lanes[2], sel.lanes[3]);
}
