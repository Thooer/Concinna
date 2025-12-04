use cap_algorithms::*;
use cap_memory::*;
use cap_containers::*;
use cap_math::*;

fn main() {
    let mut frame = FrameAllocatorResource::new(16<<20);
    let a = Allocator::new(&mut frame);

    // Test find_if on slice
    let arr: [u32; 8] = [3, 5, 7, 9, 11, 13, 2, 4];
    let pos = find_if_slice(&arr, |v| v % 2 == 0).unwrap();
    assert_eq!(pos, 6);

    // Test find_if on Vector
    let mut v = Vector::<u32>::with_capacity(a, 8).unwrap();
    for &x in arr.iter() { v.push(x).unwrap(); }
    let pos_v = find_if_vector(&v, |v| v > 10).unwrap();
    assert_eq!(pos_v, 4);

    // Build LBVH for simple points
    let centers = [
        Vec3::new(0.1, 0.2, 0.3),
        Vec3::new(0.9, 0.8, 0.7),
        Vec3::new(0.4, 0.5, 0.6),
        Vec3::new(0.2, 0.2, 0.2),
    ];
    let extent = Vec3::new(0.01, 0.01, 0.01);
    let mut prims = [AABB::from_center_extent(centers[0], extent), AABB::from_center_extent(centers[1], extent), AABB::from_center_extent(centers[2], extent), AABB::from_center_extent(centers[3], extent)];

    let needed = centers.len() * 2 - 1;
    let mut nodes = vec![LBVHNode { left: -1, right: -1, parent: -1, bbox: AABB::from_center_extent(Vec3::new(0.0,0.0,0.0), Vec3::new(0.0,0.0,0.0)) }; needed];
    let built = build_lbvh(a, &prims, &centers, &mut nodes).unwrap();
    assert_eq!(built, needed);

    let root = built - 1;
    assert!(nodes[root].left >= 0);
    // simple sanity: all leaves must have parent set
    for i in 0..centers.len() { assert!(nodes[i].parent >= 0); }

    println!("ok: lbvh nodes {}", built);
}

