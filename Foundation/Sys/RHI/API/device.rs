use std::ffi::c_void;
use prm_window::WindowHandle;
use crate::{RhiError};
use crate::resource::Model;

enum BackendKind { Vulkan, Cpu }

pub struct Device { p: *mut c_void, kind: BackendKind }

impl Device {
    pub fn new(hwnd: WindowHandle, width: u32, height: u32) -> Result<Device, RhiError> {
        if std::env::var("CONCINNA_RHI").ok().map(|v| v.eq_ignore_ascii_case("cpu")).unwrap_or(false) {
            let c = crate::backend::cpu::cpu_ctx_init(hwnd, width, height)?;
            Ok(Device { p: Box::into_raw(Box::new(c)) as *mut c_void, kind: BackendKind::Cpu })
        } else if let Ok(ctx) = crate::backend::vulkan::vk_renderer::vk_ctx_init(hwnd, width, height) {
            Ok(Device { p: Box::into_raw(Box::new(ctx)) as *mut c_void, kind: BackendKind::Vulkan })
        } else {
            let c = crate::backend::cpu::cpu_ctx_init(hwnd, width, height)?;
            Ok(Device { p: Box::into_raw(Box::new(c)) as *mut c_void, kind: BackendKind::Cpu })
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
}
