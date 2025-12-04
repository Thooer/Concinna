pub struct FiberStackPool { pub default_stack_size: usize }

impl FiberStackPool {
    pub fn new(default_stack_size: usize) -> Self { Self { default_stack_size } }
    pub fn stack_size(&self) -> usize { self.default_stack_size }
}
