use crate::Mat4;

#[repr(C)]
#[derive(Clone, Copy, Debug)]
pub struct InstanceData {
    pub model_matrix: Mat4,
    pub mesh_handle: u32,
}

#[derive(Clone)]
pub struct FramePacket {
    pub view: Mat4,
    pub proj: Mat4,
    pub instances: Vec<InstanceData>,
}

impl Default for FramePacket {
    fn default() -> Self {
        Self {
            view: Mat4([0.0; 16]),
            proj: Mat4([0.0; 16]),
            instances: Vec::new(),
        }
    }
}
