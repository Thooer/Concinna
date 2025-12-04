use core::ffi::c_void;
use cap_memory::{Allocator, MemoryBlock, MemoryError};
use crate::task::{TaskHandle, TaskFunc, ThreadAffinity, Trigger};
use sys_job::{Job, Scheduler, TaskGroup};

struct TaskNode { f: TaskFunc, arg: *mut c_void, qos: u8, affinity: ThreadAffinity, trigger: Option<Trigger> }
struct Edge { from: usize, to: usize }

pub struct TaskGraph<'a> {
    nodes: *mut TaskNode,
    edges: *mut Edge,
    node_cap: usize,
    edge_cap: usize,
    node_count: usize,
    edge_count: usize,
    nodes_blk: MemoryBlock,
    edges_blk: MemoryBlock,
    alloc: Allocator<'a>,
    next_roots: *mut usize,
    next_roots_cap: usize,
    next_roots_count: usize,
    next_roots_blk: MemoryBlock,
    dyn_indeg: *mut usize,
    dyn_blk: MemoryBlock,
    pcsc_commit_idx: usize,
    pcsc_computes: *mut usize,
    pcsc_computes_cap: usize,
    pcsc_computes_count: usize,
    pcsc_computes_blk: MemoryBlock,
}

impl<'a> TaskGraph<'a> {
    pub fn reserve(mut alloc: Allocator<'a>, nodes: usize, edges: usize) -> Result<Self, MemoryError> {
        let n_bytes = nodes.checked_mul(core::mem::size_of::<TaskNode>()).ok_or(MemoryError::Failed)?;
        let e_bytes = edges.checked_mul(core::mem::size_of::<Edge>()).ok_or(MemoryError::Failed)?;
        let n_blk = if n_bytes > 0 { alloc.alloc(n_bytes, core::mem::align_of::<TaskNode>())? } else { MemoryBlock::empty() };
        let e_blk = if e_bytes > 0 { alloc.alloc(e_bytes, core::mem::align_of::<Edge>())? } else { MemoryBlock::empty() };
        let nodes_ptr = if n_bytes > 0 { n_blk.ptr.cast::<TaskNode>() } else { core::ptr::null_mut() };
        let edges_ptr = if e_bytes > 0 { e_blk.ptr.cast::<Edge>() } else { core::ptr::null_mut() };
        let r_bytes = nodes.checked_mul(core::mem::size_of::<usize>()).ok_or(MemoryError::Failed)?;
        let r_blk = if r_bytes > 0 { alloc.alloc(r_bytes, core::mem::align_of::<usize>())? } else { MemoryBlock::empty() };
        let r_ptr = if r_bytes > 0 { r_blk.ptr.cast::<usize>() } else { core::ptr::null_mut() };
        Ok(Self { nodes: nodes_ptr, edges: edges_ptr, node_cap: nodes, edge_cap: edges, node_count: 0, edge_count: 0, nodes_blk: n_blk, edges_blk: e_blk, alloc, next_roots: r_ptr, next_roots_cap: nodes, next_roots_count: 0, next_roots_blk: r_blk, dyn_indeg: core::ptr::null_mut(), dyn_blk: MemoryBlock::empty(), pcsc_commit_idx: usize::MAX, pcsc_computes: core::ptr::null_mut(), pcsc_computes_cap: 0, pcsc_computes_count: 0, pcsc_computes_blk: MemoryBlock::empty() })
    }
    pub fn add(&mut self, f: TaskFunc, arg: *mut c_void, qos: u8, affinity: ThreadAffinity) -> TaskHandle {
        if f as usize == 0 { return TaskHandle { id: 0 }; }
        if self.node_count >= self.node_cap { return TaskHandle { id: 0 }; }
        let idx = self.node_count;
        unsafe { self.nodes.add(idx).write(TaskNode { f, arg, qos, affinity, trigger: None }); }
        self.node_count += 1;
        TaskHandle { id: idx + 1 }
    }
    pub fn set_trigger(&mut self, src: TaskHandle, t: Trigger) -> bool {
        if src.id == 0 { return false; }
        match t {
            Trigger::NextTask(dst_id) => {
                if dst_id == 0 { return false; }
                let dst = TaskHandle { id: dst_id };
                let ok = self.depends_on(dst, src);
                if ok { unsafe { let mut n = self.nodes.add(src.id - 1).read(); n.trigger = Some(Trigger::NextTask(dst_id)); self.nodes.add(src.id - 1).write(n); } }
                ok
            }
            Trigger::NextFrameRoot(dst_id) => {
                unsafe { let mut n = self.nodes.add(src.id - 1).read(); n.trigger = Some(Trigger::NextFrameRoot(dst_id)); self.nodes.add(src.id - 1).write(n); }
                true
            }
        }
    }
    pub fn depends_on(&mut self, a: TaskHandle, b: TaskHandle) -> bool {
        if a.id == 0 || b.id == 0 { return false; }
        if self.edge_count >= self.edge_cap { return false; }
        let idx = self.edge_count;
        unsafe { self.edges.add(idx).write(Edge { from: b.id - 1, to: a.id - 1 }); }
        self.edge_count += 1;
        true
    }
    pub fn chain_frames(&mut self, src: TaskHandle, dst: TaskHandle) -> bool { self.depends_on(dst, src) }
    pub fn set_arg(&mut self, h: TaskHandle, arg: *mut c_void) -> bool {
        if h.id == 0 { return false; }
        unsafe { let mut n = self.nodes.add(h.id - 1).read(); n.arg = arg; self.nodes.add(h.id - 1).write(n); }
        true
    }
    pub fn pcsc_set_commit(&mut self, h: TaskHandle) -> bool {
        if h.id == 0 { return false; }
        self.pcsc_commit_idx = h.id - 1;
        true
    }
    pub fn pcsc_add_compute(&mut self, h: TaskHandle) -> bool {
        if h.id == 0 { return false; }
        if self.pcsc_computes.is_null() {
            let bytes = self.node_cap.checked_mul(core::mem::size_of::<usize>()).unwrap();
            match self.alloc.alloc(bytes, core::mem::align_of::<usize>()) {
                Ok(b) => { self.pcsc_computes_blk = b; self.pcsc_computes = b.ptr.cast::<usize>(); self.pcsc_computes_cap = self.node_cap; }
                Err(_) => return false,
            }
        }
        if self.pcsc_computes_count >= self.pcsc_computes_cap { return false; }
        unsafe { self.pcsc_computes.add(self.pcsc_computes_count).write(h.id - 1); }
        self.pcsc_computes_count += 1;
        true
    }
    pub fn pcsc_finalize_edges(&mut self) -> bool {
        if self.pcsc_commit_idx == usize::MAX { return true; }
        if self.pcsc_commit_idx >= self.node_cap { return false; }
        unsafe {
            for i in 0..self.pcsc_computes_count {
                if self.edge_count >= self.edge_cap { return false; }
                let src_idx = self.pcsc_computes.add(i).read();
                self.edges.add(self.edge_count).write(Edge { from: src_idx, to: self.pcsc_commit_idx });
                self.edge_count += 1;
            }
        }
        true
    }
    fn enqueue_node(&self, sched: &Scheduler, tg: &TaskGroup, idx: usize) {
        unsafe {
            let wc = sched.worker_count();
            let n = self.nodes.add(idx).read();
            let qos = match n.affinity { ThreadAffinity::Main => 0usize, ThreadAffinity::Any => 0usize, ThreadAffinity::Compute(k) => { if wc > 1 { 1 + ((k as usize) % (wc - 1)) } else { 0usize } } } as u8;
            #[repr(C)]
            struct TaskCtx { g: *mut c_void, node: usize, tg: *const TaskGroup, sched: *const c_void }
            fn job_trampoline(arg: *mut c_void) {
                unsafe {
                    let ctx: Box<TaskCtx> = Box::from_raw(arg as *mut TaskCtx);
                    let g = ctx.g as *mut TaskGraph<'static>;
                    let idx = ctx.node;
                    let tg = &*ctx.tg;
                    let sched = &*(ctx.sched as *const Scheduler<'static>);
                    let n = (*g).nodes.add(idx).read();
                    (n.f)(n.arg);
                    tg.task_done();
                    for k in 0..(*g).edge_count {
                        let e = (*g).edges.add(k).read();
                        if e.from == idx {
                            let p = (*g).dyn_indeg.add(e.to);
                            let dv = p.read();
                            if dv > 0 {
                                p.write(dv - 1);
                                if dv - 1 == 0 {
                                    let wc = sched.worker_count();
                                    let sn = (*g).nodes.add(e.to).read();
                                    let qos = match sn.affinity { ThreadAffinity::Main => 0usize, ThreadAffinity::Any => 0usize, ThreadAffinity::Compute(k) => { if wc > 1 { 1 + ((k as usize) % (wc - 1)) } else { 0usize } } } as u8;
                                    let next_ctx = Box::new(TaskCtx { g: g as *mut c_void, node: e.to, tg: tg as *const TaskGroup, sched: sched as *const _ as *const c_void });
                                    let next_arg = Box::into_raw(next_ctx) as *mut c_void;
                                    let j = sys_job::Job { func: job_trampoline as fn(*mut c_void), arg: next_arg, qos };
                                    match sn.affinity { ThreadAffinity::Main => { let _ = sched.enqueue_high(j); } _ => { let _ = sched.enqueue(j); } }
                                    tg.add_tasks(1);
                                }
                            }
                        }
                    }
                    let nn = (*g).nodes.add(idx).read();
                    if let Some(Trigger::NextFrameRoot(dst_id)) = nn.trigger {
                        if (*g).next_roots_count < (*g).next_roots_cap {
                            (*g).next_roots.add((*g).next_roots_count).write(dst_id);
                            (*g).next_roots_count += 1;
                        }
                    }
                }
            }
            let ctx = Box::new(TaskCtx { g: self as *const _ as *mut c_void, node: idx, tg: tg as *const TaskGroup, sched: sched as *const _ as *const c_void });
            let arg = Box::into_raw(ctx) as *mut c_void;
            let j = sys_job::Job { func: job_trampoline as fn(*mut c_void), arg, qos };
            match n.affinity { ThreadAffinity::Main => { let _ = sched.enqueue_high(j); } _ => { let _ = sched.enqueue(j); } }
            tg.add_tasks(1);
        }
    }
    pub fn dispatch(&mut self, sched: &Scheduler, tg: &TaskGroup) -> bool {
        if self.node_count == 0 { return true; }
        if self.dyn_indeg.is_null() {
            let bytes = self.node_count.checked_mul(core::mem::size_of::<usize>()).unwrap();
            match self.alloc.alloc(bytes, core::mem::align_of::<usize>()) { Ok(b) => { self.dyn_blk = b; self.dyn_indeg = b.ptr.cast::<usize>(); } Err(_) => return false }
        }
        unsafe {
            for i in 0..self.node_count { self.dyn_indeg.add(i).write(0); }
            for i in 0..self.edge_count { let e = self.edges.add(i).read(); let v = self.dyn_indeg.add(e.to).read(); self.dyn_indeg.add(e.to).write(v + 1); }
            for i in 0..self.node_count { if self.dyn_indeg.add(i).read() == 0 { self.enqueue_node(sched, tg, i); } }
        }
        true
    }
    pub fn dispatch_next_frame_roots(&mut self, sched: &Scheduler, tg: &TaskGroup) -> usize {
        if self.next_roots_count == 0 { return 0; }
        unsafe {
            let mut count = 0usize;
            for i in 0..self.next_roots_count {
                let id = self.next_roots.add(i).read();
                if id == 0 || id > self.node_count { continue; }
                let idx = id - 1;
                self.enqueue_node(sched, tg, idx);
                count += 1;
            }
            self.next_roots_count = 0;
            count
        }
    }
    pub fn dispatch_inline(&mut self, tg: &TaskGroup) -> bool {
        if self.node_count == 0 { return true; }
        let indeg_bytes = self.node_count.checked_mul(core::mem::size_of::<usize>()).unwrap();
        let indeg_blk = match self.alloc.alloc(indeg_bytes, core::mem::align_of::<usize>()) { Ok(b) => b, Err(_) => return false };
        let indeg = indeg_blk.ptr.cast::<usize>();
        unsafe {
            for i in 0..self.node_count { indeg.add(i).write(0); }
            for i in 0..self.edge_count { let e = self.edges.add(i).read(); let v = indeg.add(e.to).read(); indeg.add(e.to).write(v + 1); }
            // simple queue using raw array indices
            let q_bytes = self.node_count.checked_mul(core::mem::size_of::<usize>()).unwrap();
            let q_blk = match self.alloc.alloc(q_bytes, core::mem::align_of::<usize>()) { Ok(b) => b, Err(_) => { let _ = self.alloc.free(indeg_blk, core::mem::align_of::<usize>()); return false; } };
            let q = q_blk.ptr.cast::<usize>();
            let mut qh = 0usize; let mut qt = 0usize;
            for i in 0..self.node_count { if indeg.add(i).read() == 0 { q.add(qt).write(i); qt += 1; } }
            while qh < qt {
                let idx = q.add(qh).read(); qh += 1;
                tg.add_tasks(1);
                let n = self.nodes.add(idx).read();
                (n.f)(n.arg);
                tg.task_done();
                for k in 0..self.edge_count { let e = self.edges.add(k).read(); if e.from == idx { let dv = indeg.add(e.to).read(); if dv > 0 { indeg.add(e.to).write(dv - 1); if dv - 1 == 0 { q.add(qt).write(e.to); qt += 1; } } } }
                if let Some(Trigger::NextFrameRoot(dst_id)) = n.trigger { if self.next_roots_count < self.next_roots_cap { self.next_roots.add(self.next_roots_count).write(dst_id); self.next_roots_count += 1; } }
            }
            let _ = self.alloc.free(q_blk, core::mem::align_of::<usize>());
        }
        let _ = self.alloc.free(indeg_blk, core::mem::align_of::<usize>());
        true
    }
}

impl<'a> Drop for TaskGraph<'a> {
    fn drop(&mut self) {
        if !self.nodes_blk.is_empty() { let _ = self.alloc.free(self.nodes_blk, core::mem::align_of::<TaskNode>()); }
        if !self.edges_blk.is_empty() { let _ = self.alloc.free(self.edges_blk, core::mem::align_of::<Edge>()); }
        if !self.next_roots_blk.is_empty() { let _ = self.alloc.free(self.next_roots_blk, core::mem::align_of::<usize>()); }
        if !self.dyn_blk.is_empty() { let _ = self.alloc.free(self.dyn_blk, core::mem::align_of::<usize>()); }
    }
}
