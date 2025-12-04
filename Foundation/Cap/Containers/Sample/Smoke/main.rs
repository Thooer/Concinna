use cap_memory::*;
use cap_containers::*;
use prm_threading::*;

fn main() {
    let mut stack = StackAllocatorResource::new(4<<20);
    let a = Allocator::new(&mut stack);
    let mut v = Vector::with_capacity(a, 4).unwrap();
    for i in 0..10 { v.push(i).unwrap(); }
    let s = v.len();
    let last = v.pop().unwrap();
    println!("vector {} {}", s, last);

    let q: MPMCQueue<i32> = MPMCQueue::with_capacity(a, 1024).unwrap();
    fn prod(arg: *mut core::ffi::c_void) { let q: &MPMCQueue<i32> = unsafe { &*(arg as *const MPMCQueue<i32>) }; for i in 0..1000 { let _ = q.enqueue(i); } }
    fn cons(arg: *mut core::ffi::c_void) { let q: &MPMCQueue<i32> = unsafe { &*(arg as *const MPMCQueue<i32>) }; let mut c = 0; loop { if let Some(_v) = q.dequeue() { c+=1; if c==1000 { break; } } } }
    let pq = &q as *const _ as *mut _;
    let cq = &q as *const _ as *mut _;
    let t1 = thread_create(prod as ThreadFunc, pq).unwrap();
    let t2 = thread_create(cons as ThreadFunc, cq).unwrap();
    thread_join(t1).unwrap();
    thread_join(t2).unwrap();
    println!("mpmc ok");

    let mut s = cap_containers::CapString::from_str(a, "hello").unwrap();
    s.push_str(" world").unwrap();
    println!("string {}", s.as_bytes().len());

    let mut m = cap_containers::HashMap::<u64, u64>::with_capacity(a, 64).unwrap();
    m.insert(42, 99).unwrap();
    println!("hash {}", m.get(42).unwrap());

    let mut d = cap_containers::Deque::<i32>::with_capacity(a, 8).unwrap();
    for i in 0..5 { d.push_back(i).unwrap(); }
    println!("deque {} {}", d.len(), d.get(2).unwrap());
}
