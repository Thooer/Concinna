use cap_memory::*;
use cap_math::{Vec3, Quat};
use sim_scene::SimWorld;

fn main() {
    let mut stack = StackAllocatorResource::new(4<<20);
    let a = Allocator::new(&mut stack);
    let mut world = SimWorld::with_capacity(a, 32).unwrap();
    let _e = world.spawn_transform(Vec3::new(0.0,0.0,0.0), Quat::identity(), Vec3::new(1.0,1.0,1.0)).unwrap();
    println!("{}", world.len());
}

