use prm_memory::*;

fn main() {
    let ps = page_size();
    let p = reserve(ps * 2).unwrap();
    commit(p, ps, PageProtection::ReadWrite).unwrap();
    unsafe { std::ptr::write_bytes(p, 0xAB, ps) };
    protect(p, ps, PageProtection::ReadOnly).unwrap();
    decommit(p, ps).unwrap();
    release(p).unwrap();

    let h = heap_create().unwrap();
    let a = heap_alloc(h, 1024, heap_max_alignment().min(4096)).unwrap();
    unsafe { std::ptr::write_bytes(a, 0xCD, 1024) };
    heap_free(h, a).unwrap();
    heap_destroy(h).unwrap();
    println!("Memory OK");
}

