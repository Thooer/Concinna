use ash::{vk, Entry, Instance, Device};
use prm_file::{stdout_handle, write};
use ash::khr::surface::Instance as Surface;
use ash::khr::swapchain::Device as Swapchain;
use ash::vk::Handle;
use crate::{RhiError};
use naga::{front::glsl, back::spv, valid::{Capabilities, ValidationFlags}, ShaderStage, Module};
use prm_window::WindowHandle;
use prm_wsi::create_vulkan_surface;

fn cstr(s: &str) -> std::ffi::CString { std::ffi::CString::new(s).unwrap() }

fn find_mem_type(p: vk::PhysicalDeviceMemoryProperties, req: vk::MemoryRequirements, flags: vk::MemoryPropertyFlags) -> Option<u32> {
    for i in 0..p.memory_type_count { if (req.memory_type_bits & (1 << i)) != 0 && p.memory_types[i as usize].property_flags.contains(flags) { return Some(i); } }
    None
}

pub struct VkCtx {
    pub entry: Entry,
    pub instance: Instance,
    pub surface_loader: Surface,
    pub surface: vk::SurfaceKHR,
    pub phys: vk::PhysicalDevice,
    pub device: Device,
    pub queue_family: u32,
    pub queue: vk::Queue,
    pub swapchain_loader: Swapchain,
    pub swapchain: vk::SwapchainKHR,
    pub format: vk::Format,
    pub extent: vk::Extent2D,
    pub images: Vec<vk::Image>,
    pub views: Vec<vk::ImageView>,
    pub renderpass: vk::RenderPass,
    pub pipeline_layout: vk::PipelineLayout,
    pub pipeline_color_left: vk::Pipeline,
    pub pipeline_color_right: vk::Pipeline,
    pub pipeline_depth: vk::Pipeline,
    pub framebuffers: Vec<vk::Framebuffer>,
    pub cmd_pool: vk::CommandPool,
    pub cmd: vk::CommandBuffer,
    pub img_available: vk::Semaphore,
    pub render_finished: vk::Semaphore,
    pub in_flight: vk::Fence,
    pub vbuf: vk::Buffer,
    pub vmem: vk::DeviceMemory,
    pub ibuf: vk::Buffer,
    pub imem: vk::DeviceMemory,
    pub index_count: u32,
    pub first_index: u32,
    pub second_first_index: u32,
    pub left_index_count: u32,
    pub right_index_count: u32,
    pub cpu_buf: vk::Buffer,
    pub cpu_mem: vk::DeviceMemory,
    pub depth_img: vk::Image,
    pub depth_mem: vk::DeviceMemory,
    pub depth_view: vk::ImageView,
    pub query_pool: vk::QueryPool,
    pub prev_visible: [bool;2],
    pub curr_img_index: u32,
}

pub fn vk_ctx_init(hwnd: WindowHandle, width: u32, height: u32) -> Result<VkCtx, RhiError> {
    unsafe {
        let entry = Entry::load().map_err(|_| RhiError::Failed)?;
        let app = cstr("Concinna"); let eng = cstr("Concinna");
        let app_info = vk::ApplicationInfo::default().application_name(&app).engine_name(&eng).api_version(vk::make_api_version(0,1,0,0));
        let ext_surface = cstr("VK_KHR_surface");
        let ext_win32 = cstr("VK_KHR_win32_surface");
        let ext_ptrs = [ext_surface.as_ptr(), ext_win32.as_ptr()];
        let ici = vk::InstanceCreateInfo::default().application_info(&app_info).enabled_extension_names(&ext_ptrs);
        let instance = entry.create_instance(&ici, None).map_err(|_| RhiError::Failed)?;
        let _ = write(stdout_handle(), b"[VK] Instance created\n");
        let surface_loader = Surface::new(&entry, &instance);
        let raw = create_vulkan_surface(instance.handle().as_raw() as *mut std::ffi::c_void, hwnd).map_err(|_| RhiError::Failed)?;
        let _ = write(stdout_handle(), b"[VK] Surface created\n");
        let surface = vk::SurfaceKHR::from_raw(raw.0 as u64);
        let phys = instance.enumerate_physical_devices().map_err(|_| RhiError::Failed)?[0];
        let props = instance.get_physical_device_queue_family_properties(phys);
        let mut q_family = None;
        for (i,p) in props.iter().enumerate() {
            let present = surface_loader.get_physical_device_surface_support(phys, i as u32, surface).unwrap_or(false);
            if p.queue_flags.contains(vk::QueueFlags::GRAPHICS) && present { q_family = Some(i as u32); break; }
        }
        let qf = q_family.ok_or(RhiError::Failed)?;
        let priorities = [1.0];
        let qci = vk::DeviceQueueCreateInfo::default().queue_family_index(qf).queue_priorities(&priorities);
        let ext_swapchain = cstr("VK_KHR_swapchain");
        let dev_exts = [ext_swapchain.as_ptr()];
        let qcis = [qci];
        let dci = vk::DeviceCreateInfo::default().queue_create_infos(&qcis).enabled_extension_names(&dev_exts);
        let device = instance.create_device(phys, &dci, None).map_err(|_| RhiError::Failed)?;
        let _ = write(stdout_handle(), b"[VK] Device created\n");
        let queue = device.get_device_queue(qf, 0);
        let swapchain_loader = Swapchain::new(&instance, &device);
        let caps = surface_loader.get_physical_device_surface_capabilities(phys, surface).map_err(|_| RhiError::Failed)?;
        let formats = surface_loader.get_physical_device_surface_formats(phys, surface).map_err(|_| RhiError::Failed)?;
        let format = formats.iter().find(|f| f.format == vk::Format::B8G8R8A8_SRGB).cloned().unwrap_or(vk::SurfaceFormatKHR { format: vk::Format::B8G8R8A8_UNORM, color_space: vk::ColorSpaceKHR::SRGB_NONLINEAR }).format;
        let extent = if caps.current_extent.width != u32::MAX { caps.current_extent } else { vk::Extent2D { width, height } };
        let image_count = (caps.min_image_count + 1).min(if caps.max_image_count>0 { caps.max_image_count } else { u32::MAX });
        let scci = vk::SwapchainCreateInfoKHR::default().surface(surface).min_image_count(image_count).image_format(format).image_color_space(vk::ColorSpaceKHR::SRGB_NONLINEAR).image_extent(extent).image_array_layers(1).image_usage(vk::ImageUsageFlags::COLOR_ATTACHMENT | vk::ImageUsageFlags::TRANSFER_DST).image_sharing_mode(vk::SharingMode::EXCLUSIVE).pre_transform(caps.current_transform).composite_alpha(vk::CompositeAlphaFlagsKHR::OPAQUE).present_mode(vk::PresentModeKHR::FIFO).clipped(true);
        let swapchain = swapchain_loader.create_swapchain(&scci, None).map_err(|_| RhiError::Failed)?;
        let _ = write(stdout_handle(), b"[VK] Swapchain created\n");
        let images = swapchain_loader.get_swapchain_images(swapchain).map_err(|_| RhiError::Failed)?;
        let views: Vec<vk::ImageView> = images.iter().map(|&img| {
            let ici = vk::ImageViewCreateInfo::default().image(img).view_type(vk::ImageViewType::TYPE_2D).format(format).subresource_range(vk::ImageSubresourceRange { aspect_mask: vk::ImageAspectFlags::COLOR, base_mip_level: 0, level_count: 1, base_array_layer: 0, layer_count: 1 });
            device.create_image_view(&ici, None).unwrap()
        }).collect();
        let depth_format = vk::Format::D32_SFLOAT;
        let color_att = vk::AttachmentDescription::default().format(format).samples(vk::SampleCountFlags::TYPE_1).load_op(vk::AttachmentLoadOp::CLEAR).store_op(vk::AttachmentStoreOp::STORE).initial_layout(vk::ImageLayout::UNDEFINED).final_layout(vk::ImageLayout::PRESENT_SRC_KHR);
        let depth_att = vk::AttachmentDescription::default().format(depth_format).samples(vk::SampleCountFlags::TYPE_1).load_op(vk::AttachmentLoadOp::CLEAR).store_op(vk::AttachmentStoreOp::DONT_CARE).stencil_load_op(vk::AttachmentLoadOp::DONT_CARE).stencil_store_op(vk::AttachmentStoreOp::DONT_CARE).initial_layout(vk::ImageLayout::UNDEFINED).final_layout(vk::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        let color_ref = vk::AttachmentReference::default().attachment(0).layout(vk::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
        let depth_ref = vk::AttachmentReference::default().attachment(1).layout(vk::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        let color_refs = [color_ref];
        let subpass = vk::SubpassDescription::default().pipeline_bind_point(vk::PipelineBindPoint::GRAPHICS).color_attachments(&color_refs).depth_stencil_attachment(&depth_ref);
        let atts = [color_att, depth_att];
        let subpasses = [subpass];
        let rpci = vk::RenderPassCreateInfo::default().attachments(&atts).subpasses(&subpasses);
        let renderpass = device.create_render_pass(&rpci, None).map_err(|_| RhiError::Failed)?;
        let _ = write(stdout_handle(), b"[VK] RenderPass created\n");
        let depth_ci = vk::ImageCreateInfo::default().image_type(vk::ImageType::TYPE_2D).format(depth_format).extent(vk::Extent3D { width: extent.width, height: extent.height, depth: 1 }).mip_levels(1).array_layers(1).samples(vk::SampleCountFlags::TYPE_1).tiling(vk::ImageTiling::OPTIMAL).usage(vk::ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT).sharing_mode(vk::SharingMode::EXCLUSIVE).initial_layout(vk::ImageLayout::UNDEFINED);
        let depth_img = device.create_image(&depth_ci, None).map_err(|_| RhiError::Failed)?;
        let _ = write(stdout_handle(), b"[VK] Depth image created\n");
        let dreq = device.get_image_memory_requirements(depth_img);
        let pmem2 = instance.get_physical_device_memory_properties(phys);
        let mtype_d = find_mem_type(pmem2, dreq, vk::MemoryPropertyFlags::DEVICE_LOCAL).ok_or(RhiError::Failed)?;
        let depth_mem = device.allocate_memory(&vk::MemoryAllocateInfo::default().allocation_size(dreq.size).memory_type_index(mtype_d), None).map_err(|_| RhiError::Failed)?;
        device.bind_image_memory(depth_img, depth_mem, 0).map_err(|_| RhiError::Failed)?;
        let dview_ci = vk::ImageViewCreateInfo::default().image(depth_img).view_type(vk::ImageViewType::TYPE_2D).format(depth_format).subresource_range(vk::ImageSubresourceRange { aspect_mask: vk::ImageAspectFlags::DEPTH, base_mip_level: 0, level_count: 1, base_array_layer: 0, layer_count: 1 });
        let depth_view = device.create_image_view(&dview_ci, None).map_err(|_| RhiError::Failed)?;
        let framebuffers: Vec<vk::Framebuffer> = views.iter().map(|&v| {
            let binding = [v, depth_view];
            let fci = vk::FramebufferCreateInfo::default().render_pass(renderpass).attachments(&binding).width(extent.width).height(extent.height).layers(1);
            device.create_framebuffer(&fci, None).unwrap()
        }).collect();
        let cmd_pool_ci = vk::CommandPoolCreateInfo::default().queue_family_index(qf).flags(vk::CommandPoolCreateFlags::RESET_COMMAND_BUFFER);
        let cmd_pool = device.create_command_pool(&cmd_pool_ci, None).map_err(|_| RhiError::Failed)?;
        let _ = write(stdout_handle(), b"[VK] CmdPool created\n");
        let alloc = vk::CommandBufferAllocateInfo::default().command_pool(cmd_pool).level(vk::CommandBufferLevel::PRIMARY).command_buffer_count(1);
        let cmd = device.allocate_command_buffers(&alloc).map_err(|_| RhiError::Failed)?[0];
        let sem_ci = vk::SemaphoreCreateInfo::default();
        let img_available = device.create_semaphore(&sem_ci, None).map_err(|_| RhiError::Failed)?;
        let render_finished = device.create_semaphore(&sem_ci, None).map_err(|_| RhiError::Failed)?;
        let fence_ci = vk::FenceCreateInfo::default().flags(vk::FenceCreateFlags::SIGNALED);
        let in_flight = device.create_fence(&fence_ci, None).map_err(|_| RhiError::Failed)?;

        let cpu_size = (extent.width as u64) * (extent.height as u64) * 4;
        let cpu_ci = vk::BufferCreateInfo::default().size(cpu_size).usage(vk::BufferUsageFlags::TRANSFER_SRC).sharing_mode(vk::SharingMode::EXCLUSIVE);
        let cpu_buf = device.create_buffer(&cpu_ci, None).map_err(|_| RhiError::Failed)?;
        let req = device.get_buffer_memory_requirements(cpu_buf);
        let pmem = instance.get_physical_device_memory_properties(phys);
        let mtype = find_mem_type(pmem, req, vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT).ok_or(RhiError::Failed)?;
        let cpu_mem = device.allocate_memory(&vk::MemoryAllocateInfo::default().allocation_size(req.size).memory_type_index(mtype), None).map_err(|_| RhiError::Failed)?;
        device.bind_buffer_memory(cpu_buf, cpu_mem, 0).map_err(|_| RhiError::Failed)?;

        let plci = vk::PipelineLayoutCreateInfo::default();
        let pipeline_layout = device.create_pipeline_layout(&plci, None).map_err(|_| RhiError::Failed)?;
        let vsrc = "#version 450\nlayout(location=0) in vec4 in_pos;\nvoid main(){ gl_Position = in_pos; }";
        let fsrc_left = "#version 450\nlayout(location=0) out vec4 out_color;\nvoid main(){ out_color = vec4(0.25,0.75,0.25,1.0); }";
        let fsrc_right = "#version 450\nlayout(location=0) out vec4 out_color;\nvoid main(){ out_color = vec4(0.25,0.25,0.75,1.0); }";
        let mut parser = glsl::Frontend::default();
        let options = glsl::Options::from(naga::ShaderStage::Vertex);
        let mut module: Module = parser.parse(&options, vsrc).map_err(|_| RhiError::Failed)?;
        let info = naga::valid::Validator::new(ValidationFlags::all(), Capabilities::all()).validate(&module).map_err(|_| RhiError::Failed)?;
        let mut spv_writer = spv::Writer::new(&spv::Options::default()).map_err(|_| RhiError::Failed)?;
        let mut vwords: Vec<u32> = Vec::new();
        spv_writer.write(&module, &info, None, &None, &mut vwords).map_err(|_| RhiError::Failed)?;
        let options_f = glsl::Options::from(naga::ShaderStage::Fragment);
        let mut module_f_left: Module = glsl::Frontend::default().parse(&options_f, fsrc_left).map_err(|_| RhiError::Failed)?;
        let info_f_left = naga::valid::Validator::new(ValidationFlags::all(), Capabilities::all()).validate(&module_f_left).map_err(|_| RhiError::Failed)?;
        let mut spv_writer_f = spv::Writer::new(&spv::Options::default()).map_err(|_| RhiError::Failed)?;
        let mut fwords_left: Vec<u32> = Vec::new();
        spv_writer_f.write(&module_f_left, &info_f_left, None, &None, &mut fwords_left).map_err(|_| RhiError::Failed)?;
        let mut module_f_right: Module = glsl::Frontend::default().parse(&options_f, fsrc_right).map_err(|_| RhiError::Failed)?;
        let info_f_right = naga::valid::Validator::new(ValidationFlags::all(), Capabilities::all()).validate(&module_f_right).map_err(|_| RhiError::Failed)?;
        let mut spv_writer_f2 = spv::Writer::new(&spv::Options::default()).map_err(|_| RhiError::Failed)?;
        let mut fwords_right: Vec<u32> = Vec::new();
        spv_writer_f2.write(&module_f_right, &info_f_right, None, &None, &mut fwords_right).map_err(|_| RhiError::Failed)?;
        let vs = device.create_shader_module(&vk::ShaderModuleCreateInfo::default().code(vwords.as_slice()), None).map_err(|_| RhiError::Failed)?;
        let fs_left = device.create_shader_module(&vk::ShaderModuleCreateInfo::default().code(fwords_left.as_slice()), None).map_err(|_| RhiError::Failed)?;
        let fs_right = device.create_shader_module(&vk::ShaderModuleCreateInfo::default().code(fwords_right.as_slice()), None).map_err(|_| RhiError::Failed)?;
        let entry_name = cstr("main");
        let entry_cstr = entry_name.as_c_str();
        let stages_left = [
            vk::PipelineShaderStageCreateInfo::default().stage(vk::ShaderStageFlags::VERTEX).module(vs).name(entry_cstr),
            vk::PipelineShaderStageCreateInfo::default().stage(vk::ShaderStageFlags::FRAGMENT).module(fs_left).name(entry_cstr),
        ];
        let stages_right = [
            vk::PipelineShaderStageCreateInfo::default().stage(vk::ShaderStageFlags::VERTEX).module(vs).name(entry_cstr),
            vk::PipelineShaderStageCreateInfo::default().stage(vk::ShaderStageFlags::FRAGMENT).module(fs_right).name(entry_cstr),
        ];
        let bind = vk::VertexInputBindingDescription::default().binding(0).stride(16).input_rate(vk::VertexInputRate::VERTEX);
        let attr = vk::VertexInputAttributeDescription::default().location(0).binding(0).format(vk::Format::R32G32B32A32_SFLOAT).offset(0);
        let binds = [bind];
        let attrs = [attr];
        let vi = vk::PipelineVertexInputStateCreateInfo::default().vertex_binding_descriptions(&binds).vertex_attribute_descriptions(&attrs);
        let ia = vk::PipelineInputAssemblyStateCreateInfo::default().topology(vk::PrimitiveTopology::TRIANGLE_LIST);
        let vp = vk::Viewport::default().x(0.0).y(0.0).width(extent.width as f32).height(extent.height as f32).min_depth(0.0).max_depth(1.0);
        let sc = vk::Rect2D::default().offset(vk::Offset2D { x: 0, y: 0 }).extent(extent);
        let vps_viewports = [vp];
        let vps_scissors = [sc];
        let vps = vk::PipelineViewportStateCreateInfo::default().viewports(&vps_viewports).scissors(&vps_scissors);
        let rs = vk::PipelineRasterizationStateCreateInfo::default().polygon_mode(vk::PolygonMode::FILL).cull_mode(vk::CullModeFlags::NONE).front_face(vk::FrontFace::COUNTER_CLOCKWISE).line_width(1.0);
        let ms = vk::PipelineMultisampleStateCreateInfo::default().rasterization_samples(vk::SampleCountFlags::TYPE_1);
        let ds = vk::PipelineDepthStencilStateCreateInfo::default().depth_test_enable(true).depth_write_enable(true).depth_compare_op(vk::CompareOp::LESS);
        let blend_off = vk::PipelineColorBlendAttachmentState::default().blend_enable(false).color_write_mask(vk::ColorComponentFlags::R | vk::ColorComponentFlags::G | vk::ColorComponentFlags::B | vk::ColorComponentFlags::A);
        let blend_none = vk::PipelineColorBlendAttachmentState::default().blend_enable(false).color_write_mask(vk::ColorComponentFlags::empty());
        let cb_on_atts = [blend_off];
        let cb_off_atts = [blend_none];
        let cb_on = vk::PipelineColorBlendStateCreateInfo::default().attachments(&cb_on_atts);
        let cb_off = vk::PipelineColorBlendStateCreateInfo::default().attachments(&cb_off_atts);
        let pci_color_left = vk::GraphicsPipelineCreateInfo::default().stages(&stages_left).vertex_input_state(&vi).input_assembly_state(&ia).viewport_state(&vps).rasterization_state(&rs).multisample_state(&ms).depth_stencil_state(&ds).color_blend_state(&cb_on).layout(pipeline_layout).render_pass(renderpass).subpass(0);
        let pci_color_right = vk::GraphicsPipelineCreateInfo::default().stages(&stages_right).vertex_input_state(&vi).input_assembly_state(&ia).viewport_state(&vps).rasterization_state(&rs).multisample_state(&ms).depth_stencil_state(&ds).color_blend_state(&cb_on).layout(pipeline_layout).render_pass(renderpass).subpass(0);
        let pci_depth = vk::GraphicsPipelineCreateInfo::default().stages(&stages_left).vertex_input_state(&vi).input_assembly_state(&ia).viewport_state(&vps).rasterization_state(&rs).multisample_state(&ms).depth_stencil_state(&ds).color_blend_state(&cb_off).layout(pipeline_layout).render_pass(renderpass).subpass(0);
        let pipelines = device.create_graphics_pipelines(vk::PipelineCache::null(), &[pci_color_left, pci_color_right, pci_depth], None).map_err(|_| RhiError::Failed)?;
        let _ = write(stdout_handle(), b"[VK] Pipelines created\n");
        let pipeline_color_left = pipelines[0];
        let pipeline_color_right = pipelines[1];
        let pipeline_depth = pipelines[2];
        device.destroy_shader_module(vs, None);
        device.destroy_shader_module(fs_left, None);
        device.destroy_shader_module(fs_right, None);
        let qp_ci = vk::QueryPoolCreateInfo::default().query_type(vk::QueryType::OCCLUSION).query_count(2);
        let query_pool = device.create_query_pool(&qp_ci, None).map_err(|_| RhiError::Failed)?;
        let _ = write(stdout_handle(), b"[VK] QueryPool created\n");

        Ok(VkCtx { entry, instance, surface_loader, surface, phys, device, queue_family: qf, queue, swapchain_loader, swapchain, format, extent, images, views, renderpass, pipeline_layout, pipeline_color_left, pipeline_color_right, pipeline_depth, framebuffers, cmd_pool, cmd, img_available, render_finished, in_flight, vbuf: vk::Buffer::null(), vmem: vk::DeviceMemory::null(), ibuf: vk::Buffer::null(), imem: vk::DeviceMemory::null(), index_count: 0, first_index: 0, second_first_index: 0, left_index_count: 0, right_index_count: 0, cpu_buf, cpu_mem, depth_img, depth_mem, depth_view, query_pool, prev_visible: [true,true], curr_img_index: 0 })
    }
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
        let mtype_v = find_mem_type(pmem, vreq, vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT).ok_or(RhiError::Failed)?;
        let mtype_i = find_mem_type(pmem, ireq, vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT).ok_or(RhiError::Failed)?;
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
        Ok(())
    }
}

fn mat_mul(a: crate::resource::Mat4, b: crate::resource::Mat4) -> crate::resource::Mat4 { crate::resource::mul(a,b) }
fn transform(m: crate::resource::Mat4, p: [f32;3]) -> [f32;4] {
    let x = p[0]; let y = p[1]; let z = p[2];
    let r0 = m.0[0]*x + m.0[4]*y + m.0[8]*z + m.0[12];
    let r1 = m.0[1]*x + m.0[5]*y + m.0[9]*z + m.0[13];
    let r2 = m.0[2]*x + m.0[6]*y + m.0[10]*z + m.0[14];
    let r3 = m.0[3]*x + m.0[7]*y + m.0[11]*z + m.0[15];
    [r0,r1,r2,r3]
}

pub fn vk_ctx_update_cubes(ctx: &mut VkCtx, model: &crate::resource::Model, angle_rad: f32, width: u32, height: u32) -> Result<(), RhiError> {
    unsafe {
        let w = width.max(1) as f32; let h = height.max(1) as f32;
        let r = 2.2f32; let eye = [angle_rad.sin()*r, 0.8f32, angle_rad.cos()*r];
        let view = crate::resource::look_at(eye, [0.0,0.0,0.0], [0.0,1.0,0.0]);
        let proj = crate::resource::perspective(60.0f32.to_radians(), w / h, 0.1, 10.0);
        let t_left = crate::resource::Mat4([1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, -0.6,0.0,0.0,1.0]);
        let t_right = crate::resource::Mat4([1.0,0.0,0.0,0.0, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.6,0.0,0.0,1.0]);
        let vp = mat_mul(proj, view);
        let m_left = mat_mul(vp, t_left);
        let m_right = mat_mul(vp, t_right);
        let vreq = ctx.device.get_buffer_memory_requirements(ctx.vbuf);
        let vptr = ctx.device.map_memory(ctx.vmem, 0, vreq.size, vk::MemoryMapFlags::empty()).map_err(|_| RhiError::Failed)? as *mut f32;
        let mut write = vptr;
        for &v in &model.vertices {
            let c = transform(m_left, v);
            *write.add(0) = c[0]; *write.add(1) = c[1]; *write.add(2) = c[2]; *write.add(3) = c[3]; write = write.add(4);
        }
        for &v in &model.vertices {
            let c = transform(m_right, v);
            *write.add(0) = c[0]; *write.add(1) = c[1]; *write.add(2) = c[2]; *write.add(3) = c[3]; write = write.add(4);
        }
        ctx.device.unmap_memory(ctx.vmem);
        Ok(())
    }
}

pub fn vk_ctx_present_pixels(ctx: &mut VkCtx, pixels: &[u8]) -> Result<(), RhiError> {
    unsafe {
        let device = &ctx.device;
        let swap = &ctx.swapchain_loader;
        let size = (ctx.extent.width as usize * ctx.extent.height as usize * 4);
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

pub fn vk_ctx_begin_frame(ctx: &mut VkCtx) -> Result<(), RhiError> {
    unsafe {
        let _ = ctx.device.wait_for_fences(&[ctx.in_flight], true, u64::MAX);
        let _ = ctx.device.reset_fences(&[ctx.in_flight]);
        let (img_index, _) = ctx.swapchain_loader.acquire_next_image(ctx.swapchain, u64::MAX, ctx.img_available, vk::Fence::null()).map_err(|_| RhiError::Failed)?;
        ctx.curr_img_index = img_index;
        let begin = vk::CommandBufferBeginInfo::default();
        ctx.device.begin_command_buffer(ctx.cmd, &begin).map_err(|_| RhiError::Failed)?;
        let clear_color = vk::ClearValue { color: vk::ClearColorValue { float32: [0.125,0.125,0.125,1.0] } };
        let clear_depth = vk::ClearValue { depth_stencil: vk::ClearDepthStencilValue { depth: 1.0, stencil: 0 } };
        let clears = [clear_color, clear_depth];
        let rpbi = vk::RenderPassBeginInfo::default().render_pass(ctx.renderpass).framebuffer(ctx.framebuffers[img_index as usize]).render_area(vk::Rect2D { offset: vk::Offset2D { x:0, y:0 }, extent: ctx.extent }).clear_values(&clears);
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
        ctx.device.cmd_reset_query_pool(ctx.cmd, ctx.query_pool, 0, 2);
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
        if ctx.prev_visible[0] { ctx.device.cmd_draw_indexed(ctx.cmd, ctx.left_index_count, 1, ctx.first_index, 0, 0); }
        if ctx.prev_visible[1] {
            ctx.device.cmd_bind_pipeline(ctx.cmd, vk::PipelineBindPoint::GRAPHICS, ctx.pipeline_color_right);
            ctx.device.cmd_draw_indexed(ctx.cmd, ctx.right_index_count, 1, ctx.second_first_index, 0, 0);
        }
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
        ctx.swapchain_loader.queue_present(ctx.queue, &present).map_err(|_| RhiError::Failed)?;
        let mut res = [0u64;2];
        let _ = ctx.device.get_query_pool_results(ctx.query_pool, 0, &mut res, vk::QueryResultFlags::WAIT | vk::QueryResultFlags::TYPE_64);
        ctx.prev_visible[0] = res[0] != 0;
        ctx.prev_visible[1] = res[1] != 0;
        Ok(())
    }
}
