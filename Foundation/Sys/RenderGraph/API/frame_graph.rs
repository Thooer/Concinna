use crate::PassDesc;

pub struct FrameGraph<'a> {
    passes: Vec<PassDesc<'a>>,
}

impl<'a> FrameGraph<'a> {
    pub fn new() -> Self {
        Self { passes: Vec::new() }
    }

    pub fn add_pass(&mut self, pass: PassDesc<'a>) {
        self.passes.push(pass);
    }

    pub fn compile(&self) -> bool {
        !self.passes.is_empty()
    }

    pub fn execute(&mut self, device: &mut sys_rhi::Device) {
        // Simple linear execution for now.
        // Future: Topological sort based on dependencies.
        // Future: Barrier injection.
        for pass in &mut self.passes {
            if let Some(ref mut exec_fn) = pass.execute {
                exec_fn(device);
            }
        }
    }
}
