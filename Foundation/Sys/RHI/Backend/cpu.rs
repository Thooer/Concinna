use prm_window::WindowHandle;
use prm_wsi::{create_cpu_present, cpu_get_buffer, cpu_get_pitch, cpu_present, CpuPresentHandle};
use crate::{RhiError};
use crate::resource::{Model, raster_cubes_rgba};
use crate::resource::{Mat4, raster_instances_rgba};

pub struct CpuCtx {
    present: CpuPresentHandle,
    width: u32,
    height: u32,
    pixels: Vec<u8>,
}

pub fn cpu_ctx_init(hwnd: WindowHandle, width: u32, height: u32) -> Result<CpuCtx, RhiError> {
    let p = create_cpu_present(hwnd, width, height).map_err(|_| RhiError::Failed)?;
    Ok(CpuCtx { present: p, width, height, pixels: vec![0u8; (width as usize)*(height as usize)*4] })
}

pub fn cpu_ctx_upload_model(_ctx: &mut CpuCtx, _m: &Model) -> Result<(), RhiError> { Ok(()) }

pub fn cpu_ctx_update_cubes(ctx: &mut CpuCtx, m: &Model, angle: f32, width: u32, height: u32) -> Result<(), RhiError> {
    if width != ctx.width || height != ctx.height { ctx.width = width; ctx.height = height; ctx.pixels.resize((width as usize)*(height as usize)*4, 0); }
    ctx.pixels = raster_cubes_rgba(width, height, m, angle);
    Ok(())
}

pub fn cpu_ctx_update_instances(ctx: &mut CpuCtx, m: &Model, transforms: &[Mat4], view: Mat4, proj: Mat4, width: u32, height: u32) -> Result<(), RhiError> {
    if width != ctx.width || height != ctx.height { ctx.width = width; ctx.height = height; ctx.pixels.resize((width as usize)*(height as usize)*4, 0); }
    ctx.pixels = raster_instances_rgba(width, height, m, transforms, view, proj);
    Ok(())
}

pub fn cpu_ctx_begin_frame(_ctx: &mut CpuCtx) -> Result<(), RhiError> { Ok(()) }
pub fn cpu_ctx_depth_prepass(_ctx: &mut CpuCtx) -> Result<(), RhiError> { Ok(()) }
pub fn cpu_ctx_color_pass(_ctx: &mut CpuCtx) -> Result<(), RhiError> { Ok(()) }

pub fn cpu_ctx_present(ctx: &mut CpuCtx) -> Result<(), RhiError> {
    let ptr = cpu_get_buffer(ctx.present).map_err(|_| RhiError::Failed)?;
    let pitch = cpu_get_pitch(ctx.present).map_err(|_| RhiError::Failed)? as usize;
    let w = ctx.width as usize; let h = ctx.height as usize;
    let row = w * 4;
    if pitch < row { return Err(RhiError::Failed); }
    unsafe {
        for y in 0..h { let dst = ptr.add(y*pitch); let src = ctx.pixels.as_ptr().add(y*row); core::ptr::copy_nonoverlapping(src, dst, row); }
    }
    cpu_present(ctx.present).map_err(|_| RhiError::Failed)?;
    Ok(())
}
