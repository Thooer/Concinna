use std::collections::HashMap;
use std::fmt;

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct ResourceId(pub u32);

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ResourceAccess {
    None,
    Read,
    Write,
    ReadWrite,
}

#[derive(Clone, Debug)]
pub struct PassResource {
    pub id: ResourceId,
    pub access: ResourceAccess,
}

pub struct PassDesc<'a> {
    pub name: String,
    pub inputs: Vec<PassResource>,
    pub outputs: Vec<PassResource>,
    pub execute: Option<Box<dyn FnMut(&mut sys_rhi::Device) + 'a>>,
}

impl<'a> fmt::Debug for PassDesc<'a> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.debug_struct("PassDesc")
            .field("name", &self.name)
            .field("inputs", &self.inputs)
            .field("outputs", &self.outputs)
            .field("execute", &"Closure")
            .finish()
    }
}

impl<'a> PassDesc<'a> {
    pub fn new(name: &str) -> Self {
        Self {
            name: name.to_string(),
            inputs: Vec::new(),
            outputs: Vec::new(),
            execute: None,
        }
    }
    pub fn read(mut self, id: ResourceId) -> Self {
        self.inputs.push(PassResource { id, access: ResourceAccess::Read });
        self
    }
    pub fn write(mut self, id: ResourceId) -> Self {
        self.outputs.push(PassResource { id, access: ResourceAccess::Write });
        self
    }
    pub fn execute<F>(mut self, f: F) -> Self 
    where F: FnMut(&mut sys_rhi::Device) + 'a 
    {
        self.execute = Some(Box::new(f));
        self
    }
}
