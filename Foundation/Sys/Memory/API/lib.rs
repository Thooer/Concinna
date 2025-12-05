use cap_memory::{Allocator, MemoryError, StackAllocatorResource};

pub struct StateRingResource {
    arena: StackAllocatorResource,
    marks: Vec<usize>,
    max_frames: usize,
    cur_begin: usize,
}

impl StateRingResource {
    pub fn new(capacity: usize, max_frames: usize) -> Self {
        let m = if max_frames == 0 { 1 } else { max_frames };
        let a = StackAllocatorResource::new(capacity);
        let b = a.marker();
        Self { arena: a, marks: Vec::with_capacity(m), max_frames: m, cur_begin: b }
    }
    pub fn allocator<'a>(&'a mut self) -> Allocator<'a> { Allocator::new(&mut self.arena) }
    pub fn begin_frame(&mut self) { self.cur_begin = self.arena.marker(); }
    pub fn commit_frame(&mut self) {
        let m = self.arena.marker();
        if self.marks.len() == self.max_frames { let _ = self.marks.remove(0); }
        self.marks.push(m);
        self.cur_begin = m;
    }
    pub fn rollback_to_index(&mut self, index: usize) -> bool {
        if index >= self.marks.len() { return false; }
        let m = self.marks[index];
        self.arena.reset_to(m);
        self.marks.truncate(index + 1);
        self.cur_begin = m;
        true
    }
    pub fn reset_current(&mut self) { self.arena.reset_to(self.cur_begin); }
    pub fn frame_count(&self) -> usize { self.marks.len() }
    pub fn latest_index(&self) -> Option<usize> { if self.marks.is_empty() { None } else { Some(self.marks.len() - 1) } }
}
