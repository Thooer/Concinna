use ash::{vk};
use prm_time::now;
use crate::RhiError;
use super::vk_renderer::VkCtx;

pub fn vk_ctx_begin_frame(ctx: &mut VkCtx) -> Result<(), RhiError> {
    unsafe {
        let _ = ctx.device.wait_for_fences(&[ctx.in_flight], true, u64::MAX);
        let _ = ctx.device.reset_fences(&[ctx.in_flight]);
        match ctx.swapchain_loader.acquire_next_image(ctx.swapchain, u64::MAX, ctx.img_available, vk::Fence::null()) {
            Ok((img_index, sub)) => { ctx.curr_img_index = img_index; if sub { let _ = super::vk_renderer::vk_ctx_recreate(ctx, ctx.extent.width, ctx.extent.height); } }
            Err(_) => { super::vk_renderer::vk_ctx_recreate(ctx, ctx.extent.width, ctx.extent.height)?; let (img_index, _) = ctx.swapchain_loader.acquire_next_image(ctx.swapchain, u64::MAX, ctx.img_available, vk::Fence::null()).map_err(|_| RhiError::Failed)?; ctx.curr_img_index = img_index; }
        }
        let begin = vk::CommandBufferBeginInfo::default();
        ctx.device.begin_command_buffer(ctx.cmd, &begin).map_err(|_| RhiError::Failed)?;
        ctx.device.cmd_reset_query_pool(ctx.cmd, ctx.query_pool, 0, 2);
        let clear_color = vk::ClearValue { color: vk::ClearColorValue { float32: [0.125,0.125,0.125,1.0] } };
        let clear_depth = vk::ClearValue { depth_stencil: vk::ClearDepthStencilValue { depth: 1.0, stencil: 0 } };
        let clears = [clear_color, clear_depth];
        let rpbi = vk::RenderPassBeginInfo::default().render_pass(ctx.renderpass).framebuffer(ctx.framebuffers[ctx.curr_img_index as usize]).render_area(vk::Rect2D { offset: vk::Offset2D { x:0, y:0 }, extent: ctx.extent }).clear_values(&clears);
        ctx.device.cmd_begin_render_pass(ctx.cmd, &rpbi, vk::SubpassContents::INLINE);
        Ok(())
    }
}

pub fn vk_ctx_depth_prepass(ctx: &mut VkCtx) -> Result<(), RhiError> {
    unsafe {
        ctx.device.cmd_bind_pipeline(ctx.cmd, vk::PipelineBindPoint::GRAPHICS, ctx.pipeline_depth);
        let vb = [ctx.vbuf]; let offs = [0u64];
        ctx.device.cmd_bind_vertex_buffers(ctx.cmd, 0, &vb, &offs);
        ctx.device.cmd_bind_index_buffer(ctx.cmd, ctx.ibuf, 0, vk::IndexType::UINT32);
        ctx.device.cmd_begin_query(ctx.cmd, ctx.query_pool, 0, vk::QueryControlFlags::empty());
        ctx.device.cmd_draw_indexed(ctx.cmd, ctx.left_index_count, 1, ctx.first_index, 0, 0);
        ctx.device.cmd_end_query(ctx.cmd, ctx.query_pool, 0);
        ctx.device.cmd_begin_query(ctx.cmd, ctx.query_pool, 1, vk::QueryControlFlags::empty());
        ctx.device.cmd_draw_indexed(ctx.cmd, ctx.right_index_count, 1, ctx.second_first_index, 0, 0);
        ctx.device.cmd_end_query(ctx.cmd, ctx.query_pool, 1);
        Ok(())
    }
}

pub fn vk_ctx_color_pass(ctx: &mut VkCtx) -> Result<(), RhiError> {
    unsafe {
        ctx.device.cmd_bind_pipeline(ctx.cmd, vk::PipelineBindPoint::GRAPHICS, ctx.pipeline_color_left);
        let vb = [ctx.vbuf]; let offs = [0u64];
        ctx.device.cmd_bind_vertex_buffers(ctx.cmd, 0, &vb, &offs);
        ctx.device.cmd_bind_index_buffer(ctx.cmd, ctx.ibuf, 0, vk::IndexType::UINT32);
        ctx.device.cmd_draw_indexed(ctx.cmd, ctx.left_index_count, 1, ctx.first_index, 0, 0);
        ctx.device.cmd_bind_pipeline(ctx.cmd, vk::PipelineBindPoint::GRAPHICS, ctx.pipeline_color_right);
        ctx.device.cmd_draw_indexed(ctx.cmd, ctx.right_index_count, 1, ctx.second_first_index, 0, 0);
        Ok(())
    }
}

pub fn vk_ctx_present_and_update_queries(ctx: &mut VkCtx) -> Result<(), RhiError> {
    unsafe {
        ctx.device.cmd_end_render_pass(ctx.cmd);
        ctx.device.end_command_buffer(ctx.cmd).map_err(|_| RhiError::Failed)?;
        let waits = [ctx.img_available];
        let stages = [vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT];
        let cmds = [ctx.cmd];
        let sigs = [ctx.render_finished];
        let submit = vk::SubmitInfo::default().wait_semaphores(&waits).wait_dst_stage_mask(&stages).command_buffers(&cmds).signal_semaphores(&sigs);
        ctx.device.queue_submit(ctx.queue, &[submit], ctx.in_flight).map_err(|_| RhiError::Failed)?;
        let swaps = [ctx.swapchain]; let idxs = [ctx.curr_img_index];
        let present = vk::PresentInfoKHR::default().wait_semaphores(&sigs).swapchains(&swaps).image_indices(&idxs);
        match ctx.swapchain_loader.queue_present(ctx.queue, &present) {
            Ok(_) => {}
            Err(_) => { let _ = super::vk_renderer::vk_ctx_recreate(ctx, ctx.extent.width, ctx.extent.height); }
        }
        let mut results: [u64;2] = [0,0];
        let flags = vk::QueryResultFlags::WAIT | vk::QueryResultFlags::TYPE_64;
        let _ = ctx.device.get_query_pool_results(ctx.query_pool, 0, &mut results, flags);
        ctx.prev_visible[0] = results[0] > 0;
        ctx.prev_visible[1] = results[1] > 0;
        static mut LAST_QUERY_LOG_NS: i64 = 0;
        let t = now();
        if LAST_QUERY_LOG_NS == 0 || t - LAST_QUERY_LOG_NS >= 1_000_000_000 {
            let _ = prm_file::write(prm_file::stdout_handle(), format!("[VK] Visible L={} R={}\n", ctx.prev_visible[0] as u8, ctx.prev_visible[1] as u8).as_bytes());
            LAST_QUERY_LOG_NS = t;
        }
        Ok(())
    }
}

pub fn vk_ctx_present_pixels(ctx: &mut VkCtx, pixels: &[u8]) -> Result<(), RhiError> {
    unsafe {
        let device = &ctx.device;
        let swap = &ctx.swapchain_loader;
        let size = ctx.extent.width as usize * ctx.extent.height as usize * 4;
        if pixels.len() != size { return Err(RhiError::Failed); }
        let ptr = device.map_memory(ctx.cpu_mem, 0, size as u64, vk::MemoryMapFlags::empty()).map_err(|_| RhiError::Failed)? as *mut u8;
        core::ptr::copy_nonoverlapping(pixels.as_ptr(), ptr, size);
        device.unmap_memory(ctx.cpu_mem);
        let _ = device.wait_for_fences(&[ctx.in_flight], true, u64::MAX);
        let _ = device.reset_fences(&[ctx.in_flight]);
        let (img_index, _) = swap.acquire_next_image(ctx.swapchain, u64::MAX, ctx.img_available, vk::Fence::null()).map_err(|_| RhiError::Failed)?;
        let image = ctx.images[img_index as usize];
        let begin = vk::CommandBufferBeginInfo::default();
        device.begin_command_buffer(ctx.cmd, &begin).map_err(|_| RhiError::Failed)?;
        let barrier_to_copy = vk::ImageMemoryBarrier::default()
            .src_access_mask(vk::AccessFlags::empty())
            .dst_access_mask(vk::AccessFlags::TRANSFER_WRITE)
            .old_layout(vk::ImageLayout::UNDEFINED)
            .new_layout(vk::ImageLayout::TRANSFER_DST_OPTIMAL)
            .src_queue_family_index(vk::QUEUE_FAMILY_IGNORED)
            .dst_queue_family_index(vk::QUEUE_FAMILY_IGNORED)
            .image(image)
            .subresource_range(vk::ImageSubresourceRange { aspect_mask: vk::ImageAspectFlags::COLOR, base_mip_level: 0, level_count: 1, base_array_layer: 0, layer_count: 1 });
        device.cmd_pipeline_barrier(ctx.cmd, vk::PipelineStageFlags::TOP_OF_PIPE, vk::PipelineStageFlags::TRANSFER, vk::DependencyFlags::empty(), &[], &[], &[barrier_to_copy]);
        let sub = vk::ImageSubresourceLayers { aspect_mask: vk::ImageAspectFlags::COLOR, mip_level: 0, base_array_layer: 0, layer_count: 1 };
        let region = vk::BufferImageCopy::default().buffer_offset(0).buffer_row_length(0).buffer_image_height(0).image_subresource(sub).image_offset(vk::Offset3D { x: 0, y: 0, z: 0 }).image_extent(vk::Extent3D { width: ctx.extent.width, height: ctx.extent.height, depth: 1 });
        device.cmd_copy_buffer_to_image(ctx.cmd, ctx.cpu_buf, image, vk::ImageLayout::TRANSFER_DST_OPTIMAL, &[region]);
        let barrier_to_present = vk::ImageMemoryBarrier::default()
            .src_access_mask(vk::AccessFlags::TRANSFER_WRITE)
            .dst_access_mask(vk::AccessFlags::empty())
            .old_layout(vk::ImageLayout::TRANSFER_DST_OPTIMAL)
            .new_layout(vk::ImageLayout::PRESENT_SRC_KHR)
            .src_queue_family_index(vk::QUEUE_FAMILY_IGNORED)
            .dst_queue_family_index(vk::QUEUE_FAMILY_IGNORED)
            .image(image)
            .subresource_range(vk::ImageSubresourceRange { aspect_mask: vk::ImageAspectFlags::COLOR, base_mip_level: 0, level_count: 1, base_array_layer: 0, layer_count: 1 });
        device.cmd_pipeline_barrier(ctx.cmd, vk::PipelineStageFlags::TRANSFER, vk::PipelineStageFlags::BOTTOM_OF_PIPE, vk::DependencyFlags::empty(), &[], &[], &[barrier_to_present]);
        device.end_command_buffer(ctx.cmd).map_err(|_| RhiError::Failed)?;
        let wait_semaphores = [ctx.img_available];
        let wait_stages = [vk::PipelineStageFlags::TRANSFER];
        let cmd_bufs = [ctx.cmd];
        let signal_semaphores = [ctx.render_finished];
        let submit = vk::SubmitInfo::default().wait_semaphores(&wait_semaphores).wait_dst_stage_mask(&wait_stages).command_buffers(&cmd_bufs).signal_semaphores(&signal_semaphores);
        device.queue_submit(ctx.queue, &[submit], ctx.in_flight).map_err(|_| RhiError::Failed)?;
        let swapchains = [ctx.swapchain]; let indices = [img_index];
        let present = vk::PresentInfoKHR::default().wait_semaphores(&signal_semaphores).swapchains(&swapchains).image_indices(&indices);
        swap.queue_present(ctx.queue, &present).map_err(|_| RhiError::Failed)?;
        Ok(())
    }
}
