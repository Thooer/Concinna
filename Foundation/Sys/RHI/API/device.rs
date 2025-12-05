use std::ffi::c_void;
use prm_window::WindowHandle;
use crate::{RhiError, FramePacket};
use crate::resource::Model;
use crate::resource::Mat4;

pub enum BackendKind { Vulkan, Cpu }

pub struct Device { p: *mut c_void, kind: BackendKind }

impl Device {
    pub fn new(hwnd: WindowHandle, width: u32, height: u32, kind: BackendKind) -> Result<Device, RhiError> {
        match kind {
            BackendKind::Cpu => {
                let c = crate::backend::cpu::cpu_ctx_init(hwnd, width, height)?;
                Ok(Device { p: Box::into_raw(Box::new(c)) as *mut c_void, kind: BackendKind::Cpu })
            },
            BackendKind::Vulkan => {
                if let Ok(ctx) = crate::backend::vulkan::vk_renderer::vk_ctx_init(hwnd, width, height) {
                    Ok(Device { p: Box::into_raw(Box::new(ctx)) as *mut c_void, kind: BackendKind::Vulkan })
                } else {
                    // Fallback or error? For now, let's fail if Vulkan requested but fails
                    Err(RhiError::Failed)
                }
            }
        }
    }

    fn vk<'a>(&'a mut self) -> &'a mut crate::backend::vulkan::vk_renderer::VkCtx { unsafe { &mut *(self.p as *mut crate::backend::vulkan::vk_renderer::VkCtx) } }
    fn cpu<'a>(&'a mut self) -> &'a mut crate::backend::cpu::CpuCtx { unsafe { &mut *(self.p as *mut crate::backend::cpu::CpuCtx) } }

    pub fn upload_model(&mut self, m: &Model) -> Result<(), RhiError> {
        match self.kind {
            BackendKind::Vulkan => crate::backend::vulkan::vk_model::vk_ctx_upload_model(self.vk(), m),
            BackendKind::Cpu => crate::backend::cpu::cpu_ctx_upload_model(self.cpu(), m),
        }
    }
    pub fn render_model_instances(&mut self, m: &Model, transforms: &[Mat4], view: Mat4, proj: Mat4, width: u32, height: u32) -> Result<(), RhiError> {
        match self.kind {
            BackendKind::Vulkan => crate::backend::vulkan::vk_model::vk_ctx_update_instances(self.vk(), m, transforms, view, proj),
            BackendKind::Cpu => crate::backend::cpu::cpu_ctx_update_instances(self.cpu(), m, transforms, view, proj, width, height),
        }
    }
    pub fn update_cubes(&mut self, m: &Model, angle: f32, width: u32, height: u32) -> Result<(), RhiError> {
        match self.kind {
            BackendKind::Vulkan => crate::backend::vulkan::vk_model::vk_ctx_update_cubes(self.vk(), m, angle, width, height),
            BackendKind::Cpu => crate::backend::cpu::cpu_ctx_update_cubes(self.cpu(), m, angle, width, height),
        }
    }
    pub fn begin_frame(&mut self) -> Result<(), RhiError> {
        match self.kind {
            BackendKind::Vulkan => crate::backend::vulkan::vk_passes::vk_ctx_begin_frame(self.vk()),
            BackendKind::Cpu => crate::backend::cpu::cpu_ctx_begin_frame(self.cpu()),
        }
    }
    pub fn depth_prepass(&mut self) -> Result<(), RhiError> {
        match self.kind {
            BackendKind::Vulkan => crate::backend::vulkan::vk_passes::vk_ctx_depth_prepass(self.vk()),
            BackendKind::Cpu => crate::backend::cpu::cpu_ctx_depth_prepass(self.cpu()),
        }
    }
    pub fn color_pass(&mut self) -> Result<(), RhiError> {
        match self.kind {
            BackendKind::Vulkan => crate::backend::vulkan::vk_passes::vk_ctx_color_pass(self.vk()),
            BackendKind::Cpu => crate::backend::cpu::cpu_ctx_color_pass(self.cpu()),
        }
    }
    pub fn present(&mut self) -> Result<(), RhiError> {
        match self.kind {
            BackendKind::Vulkan => crate::backend::vulkan::vk_passes::vk_ctx_present_and_update_queries(self.vk()),
            BackendKind::Cpu => crate::backend::cpu::cpu_ctx_present(self.cpu()),
        }
    }

    pub fn create_pipeline(&mut self, desc: crate::PipelineDesc) -> Result<(), RhiError> {
        match self.kind {
            BackendKind::Vulkan => {
                let fs_right = desc.fs_right.as_deref().unwrap_or(&desc.fs);
                crate::backend::vulkan::vk_renderer::vk_ctx_create_pipelines_glsl_pair(self.vk(), &desc.vs, &desc.fs, fs_right)
            },
            BackendKind::Cpu => Ok(())
        }
    }

    pub fn draw_packet(&mut self, packet: &FramePacket, model: &Model) -> Result<(), RhiError> {
        match self.kind {
            BackendKind::Vulkan => crate::backend::vulkan::vk_model::vk_ctx_draw_packet(self.vk(), packet, model),
            BackendKind::Cpu => Ok(()) // TODO
        }
    }
}
