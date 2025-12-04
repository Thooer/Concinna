use sys_task::*;
use sys_job::*;
use cap_memory::*;
use core::ffi::c_void;
use core::sync::atomic::{AtomicUsize, Ordering};

static SUM: AtomicUsize = AtomicUsize::new(0);

fn map_job(buf_ptr: *mut c_void) {
    let buf: &mut CommandBuffer = unsafe { &mut *(buf_ptr as *mut CommandBuffer) };
    fn inc(_ctx: *mut c_void) { SUM.fetch_add(1, Ordering::Relaxed); }
    let _ = buf.push(inc as fn(*mut c_void), core::ptr::null_mut());
}
fn reduce_job(agg_ptr: *mut c_void) { let agg: &mut CommandAggregator = unsafe { &mut *(agg_ptr as *mut CommandAggregator) }; agg.apply_all(); }

fn main() {
    let mut frame = FrameAllocatorResource::new(8 << 20);
    let a = Allocator::new(&mut frame);
    let tg = TaskGroup::new();
    let mut g = TaskGraph::reserve(a, 8, 8).unwrap();
    let mut b1 = CommandBuffer::new(a, 32).unwrap();
    let mut b2 = CommandBuffer::new(a, 32).unwrap();
    let mut agg = CommandAggregator::new(a, 2).unwrap();
    let _ = agg.add(&mut b1);
    let _ = agg.add(&mut b2);
    let h_map1 = g.add(map_job as fn(*mut c_void), &mut b1 as *mut _ as *mut c_void, 0, ThreadAffinity::Compute(1));
    let h_map2 = g.add(map_job as fn(*mut c_void), &mut b2 as *mut _ as *mut c_void, 0, ThreadAffinity::Compute(1));
    let h_reduce = g.add(reduce_job as fn(*mut c_void), &mut agg as *mut _ as *mut c_void, 0, ThreadAffinity::Main);
    let _ = g.pcsc_set_commit(h_reduce);
    let _ = g.pcsc_add_compute(h_map1);
    let _ = g.pcsc_add_compute(h_map2);
    let _ = g.pcsc_finalize_edges();
    let _ = g.dispatch_inline(&tg);
    tg.wait();
    println!("sum {}", SUM.load(Ordering::Relaxed));
}
