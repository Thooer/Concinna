use std::marker::PhantomData;
use crate::RhiError;
use crate::device::Device;

pub struct Recording;
pub struct InsideRenderPass;

pub struct CommandBuffer<State> {
    dev_ptr: *mut Device,
    _marker: PhantomData<State>,
}

impl Device {
    pub fn command_buffer(&mut self) -> CommandBuffer<Recording> {
        CommandBuffer { dev_ptr: self as *mut Device, _marker: PhantomData }
    }
}

impl CommandBuffer<Recording> {
    fn dev(&mut self) -> &mut Device { unsafe { &mut *self.dev_ptr } }
    pub fn begin_frame(&mut self) -> Result<(), RhiError> { self.dev().begin_frame() }
    pub fn begin_depth_prepass(mut self) -> Result<CommandBuffer<InsideRenderPass>, RhiError> {
        self.dev().depth_prepass()?;
        Ok(CommandBuffer { dev_ptr: self.dev_ptr, _marker: PhantomData })
    }
}

impl CommandBuffer<InsideRenderPass> {
    fn dev(&mut self) -> &mut Device { unsafe { &mut *self.dev_ptr } }
    pub fn color_pass(&mut self) -> Result<(), RhiError> { self.dev().color_pass() }
    pub fn end_and_present(self) -> Result<(), RhiError> { unsafe { (&mut *self.dev_ptr).present() } }
}
