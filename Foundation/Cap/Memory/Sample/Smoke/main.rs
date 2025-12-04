use cap_memory::*;

fn main() {
    let mut res = StackAllocatorResource::new(1usize << 20);
    let a = Allocator::new(&mut res);
    let b1 = a.alloc(128, Alignment::DEFAULT).unwrap();
    let b2 = a.alloc(256, Alignment::DEFAULT).unwrap();
    a.free(b2, Alignment::DEFAULT);
    let b3 = a.alloc(256, Alignment::DEFAULT).unwrap();
    a.free(b3, Alignment::DEFAULT);
    a.free(b1, Alignment::DEFAULT);
    println!("cap_memory smoke ok {}", Alignment::DEFAULT);
}
