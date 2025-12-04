use ash::vk;
use crate::RhiError;
use super::vk_renderer::VkCtx;
use prm_time::now;

static mut LAST_LOG_NS: i64 = 0;
static mut ANGLE_T0: i64 = 0;

fn mat_mul(a: crate::resource::Mat4, b: crate::resource::Mat4) -> crate::resource::Mat4 { crate::resource::mul(a,b) }
fn transform(m: crate::resource::Mat4, p: [f32;3]) -> [f32;4] {
    let x = p[0]; let y = p[1]; let z = p[2];
    let r0 = m.0[0]*x + m.0[1]*y + m.0[2]*z + m.0[3];
    let r1 = m.0[4]*x + m.0[5]*y + m.0[6]*z + m.0[7];
    let r2 = m.0[8]*x + m.0[9]*y + m.0[10]*z + m.0[11];
    let r3 = m.0[12]*x + m.0[13]*y + m.0[14]*z + m.0[15];
    [r0,r1,r2,r3]
}

pub fn vk_ctx_upload_model(ctx: &mut VkCtx, model: &crate::resource::Model) -> Result<(), RhiError> {
    unsafe {
        if ctx.index_count != 0 { return Ok(()); }
        let vcount = model.vertices.len() as u32;
        let icount = (model.indices.len() as u32) * 3;
        let vb_sz = (vcount as u64) * 16 * 2;
        let mut indices: Vec<u32> = Vec::with_capacity((icount as usize) * 2);
        for tri in &model.indices { indices.push(tri[0]); indices.push(tri[1]); indices.push(tri[2]); }
        let first_count = indices.len() as u32;
        for tri in &model.indices { indices.push(tri[0] + vcount); indices.push(tri[1] + vcount); indices.push(tri[2] + vcount); }
        let ibytes: &[u8] = core::slice::from_raw_parts(indices.as_ptr() as *const u8, indices.len()*4);
        let vbci = vk::BufferCreateInfo::default().size(vb_sz).usage(vk::BufferUsageFlags::VERTEX_BUFFER).sharing_mode(vk::SharingMode::EXCLUSIVE);
        let ibci = vk::BufferCreateInfo::default().size(ibytes.len() as u64).usage(vk::BufferUsageFlags::INDEX_BUFFER).sharing_mode(vk::SharingMode::EXCLUSIVE);
        ctx.vbuf = ctx.device.create_buffer(&vbci, None).map_err(|_| RhiError::Failed)?;
        ctx.ibuf = ctx.device.create_buffer(&ibci, None).map_err(|_| RhiError::Failed)?;
        let vreq = ctx.device.get_buffer_memory_requirements(ctx.vbuf);
        let ireq = ctx.device.get_buffer_memory_requirements(ctx.ibuf);
        let pmem = ctx.instance.get_physical_device_memory_properties(ctx.phys);
        let mtype_v = super::vk_renderer::find_mem_type(pmem, vreq, vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT).ok_or(RhiError::Failed)?;
        let mtype_i = super::vk_renderer::find_mem_type(pmem, ireq, vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT).ok_or(RhiError::Failed)?;
        ctx.vmem = ctx.device.allocate_memory(&vk::MemoryAllocateInfo::default().allocation_size(vreq.size).memory_type_index(mtype_v), None).map_err(|_| RhiError::Failed)?;
        ctx.imem = ctx.device.allocate_memory(&vk::MemoryAllocateInfo::default().allocation_size(ireq.size).memory_type_index(mtype_i), None).map_err(|_| RhiError::Failed)?;
        ctx.device.bind_buffer_memory(ctx.vbuf, ctx.vmem, 0).map_err(|_| RhiError::Failed)?;
        ctx.device.bind_buffer_memory(ctx.ibuf, ctx.imem, 0).map_err(|_| RhiError::Failed)?;
        let iptr = ctx.device.map_memory(ctx.imem, 0, ireq.size, vk::MemoryMapFlags::empty()).map_err(|_| RhiError::Failed)? as *mut u8;
        core::ptr::copy_nonoverlapping(ibytes.as_ptr(), iptr, ibytes.len());
        ctx.device.unmap_memory(ctx.imem);
        ctx.index_count = indices.len() as u32;
        ctx.first_index = 0;
        ctx.second_first_index = first_count;
        ctx.left_index_count = first_count;
        ctx.right_index_count = ctx.index_count - first_count;
        let _ = prm_file::write(prm_file::stdout_handle(), format!("[VK] UploadModel vcount={} tri={} idx_total={} left={} right={}\n", vcount, icount/3, ctx.index_count, ctx.left_index_count, ctx.right_index_count).as_bytes());
        Ok(())
    }
}

pub fn vk_ctx_update_cubes(ctx: &mut VkCtx, model: &crate::resource::Model, angle_rad: f32, width: u32, height: u32) -> Result<(), RhiError> {
    unsafe {
        let w = width.max(1) as f32; let h = height.max(1) as f32;
        let tn = now();
        if ANGLE_T0 == 0 { ANGLE_T0 = tn; }
        let dt_ns = tn - ANGLE_T0;
        let dt_s = (dt_ns as f64) / 1_000_000_000.0;
        let angle_eff = if angle_rad.abs() < 1e-6 { (dt_s as f32) * 0.8 } else { angle_rad };
        let r = 4.0f32; let eye = [angle_eff.sin()*r, 0.8f32, angle_eff.cos()*r];
        let view = crate::resource::look_at_vk(eye, [0.0,0.0,0.0], [0.0,1.0,0.0]);
        let proj = crate::resource::perspective_vk(60.0f32.to_radians(), w / h, 0.1, 30.0);
        let t_left = crate::resource::Mat4([1.0,0.0,0.0,-1.2, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0]);
        let t_right = crate::resource::Mat4([1.0,0.0,0.0, 1.2, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0]);
        let vp = mat_mul(proj, view);
        let ca = angle_eff.cos(); let sa = angle_eff.sin();
        let rot_y = crate::resource::Mat4([
            ca, 0.0, -sa, 0.0,
            0.0, 1.0,  0.0, 0.0,
            sa, 0.0,  ca, 0.0,
            0.0, 0.0, 0.0, 1.0
        ]);
        let m_left = mat_mul(vp, crate::resource::mul(t_left, rot_y));
        let m_right = mat_mul(vp, crate::resource::mul(t_right, rot_y));
        let vreq = ctx.device.get_buffer_memory_requirements(ctx.vbuf);
        let vptr = ctx.device.map_memory(ctx.vmem, 0, vreq.size, vk::MemoryMapFlags::empty()).map_err(|_| RhiError::Failed)? as *mut f32;
        let mut write = vptr;
        let mut w_min_l = f32::MAX; let mut w_max_l = f32::MIN;
        let mut x_min_l = f32::MAX; let mut x_max_l = f32::MIN; let mut y_min_l = f32::MAX; let mut y_max_l = f32::MIN;
        for &v in &model.vertices {
            let c = transform(m_left, v);
            if c[3] < w_min_l { w_min_l = c[3]; } if c[3] > w_max_l { w_max_l = c[3]; }
            let w = c[3]; let inv_w = if w != 0.0 { 1.0 / w } else { 0.0 };
            let xn = c[0] * inv_w; let yn = c[1] * inv_w; let zn = (c[2] * inv_w) * 0.5 + 0.5;
            if xn < x_min_l { x_min_l = xn; } if xn > x_max_l { x_max_l = xn; }
            if yn < y_min_l { y_min_l = yn; } if yn > y_max_l { y_max_l = yn; }
            *write.add(0) = xn; *write.add(1) = yn; *write.add(2) = zn; *write.add(3) = 1.0; write = write.add(4);
        }
        let mut w_min_r = f32::MAX; let mut w_max_r = f32::MIN;
        let mut x_min_r = f32::MAX; let mut x_max_r = f32::MIN; let mut y_min_r = f32::MAX; let mut y_max_r = f32::MIN;
        for &v in &model.vertices {
            let c = transform(m_right, v);
            if c[3] < w_min_r { w_min_r = c[3]; } if c[3] > w_max_r { w_max_r = c[3]; }
            let w = c[3]; let inv_w = if w != 0.0 { 1.0 / w } else { 0.0 };
            let xn = c[0] * inv_w; let yn = c[1] * inv_w; let zn = (c[2] * inv_w) * 0.5 + 0.5;
            if xn < x_min_r { x_min_r = xn; } if xn > x_max_r { x_max_r = xn; }
            if yn < y_min_r { y_min_r = yn; } if yn > y_max_r { y_max_r = yn; }
            *write.add(0) = xn; *write.add(1) = yn; *write.add(2) = zn; *write.add(3) = 1.0; write = write.add(4);
        }
        ctx.device.unmap_memory(ctx.vmem);
        if LAST_LOG_NS == 0 || tn - LAST_LOG_NS >= 1_000_000_000 {
            let _ = prm_file::write(prm_file::stdout_handle(), format!("[VK] UpdateCubes done angle={:.2} (wL=[{:.3},{:.3}] wR=[{:.3},{:.3}] xL=[{:.2},{:.2}] yL=[{:.2},{:.2}] xR=[{:.2},{:.2}] yR=[{:.2},{:.2}])\n", angle_eff, w_min_l, w_max_l, w_min_r, w_max_r, x_min_l, x_max_l, y_min_l, y_max_l, x_min_r, x_max_r, y_min_r, y_max_r).as_bytes());
            LAST_LOG_NS = tn;
        }
        Ok(())
    }
}
