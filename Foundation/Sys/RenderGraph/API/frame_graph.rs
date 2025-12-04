use crate::PassDesc;
use prm_file::{stdout_handle, write};

pub struct FrameGraph { passes: Vec<PassDesc> }

impl FrameGraph {
    pub fn new() -> Self { Self { passes: Vec::new() } }
    pub fn add_pass(&mut self, name: &str) { self.passes.push(PassDesc { name: name.to_string() }); }
    pub fn compile(&self) -> bool { !self.passes.is_empty() }
    pub fn execute_with<F: FnMut(&str)>(&self, mut exec: F) {
        let h = stdout_handle();
        for p in &self.passes {
            let _ = write(h, format!("PASS {}\n", p.name).as_bytes());
            exec(&p.name);
        }
    }
}
