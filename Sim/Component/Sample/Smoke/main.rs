use cap_memory::*;
use sim_component::*;
use cap_math::{Vec3, Quat};

fn main() {
    let mut stack = StackAllocatorResource::new(2<<20);
    let a = Allocator::new(&mut stack);
    let mut ts: TransformSoA<'_, cap_containers::UseVector> = TransformSoA::with_capacity(a, 16).unwrap();
    let t = Transform::from_trs(Vec3::new(1.0,2.0,3.0), Quat::identity(), Vec3::new(1.0,1.0,1.0));
    ts.push(t).unwrap();
    println!("{}", ts.len());
}
