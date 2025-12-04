#[derive(Clone, Copy)]
pub struct F32x4 { pub lanes: [f32; 4] }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SimdError { Failed, Unsupported }

#[derive(Clone, Copy)]
pub struct Mask4 { pub lanes: [u32; 4] }

#[derive(Clone, Copy)]
pub struct F32x8 { pub lanes: [f32; 8] }

#[derive(Clone, Copy)]
pub struct Mask8 { pub lanes: [u32; 8] }
