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

pub(crate) fn find_mem_type(p: vk::PhysicalDeviceMemoryProperties, req: vk::MemoryRequirements, flags: vk::MemoryPropertyFlags) -> Option<u32> {
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

unsafe fn destroy_swapchain_related(ctx: &mut VkCtx) {
    let d = &ctx.device;
    for &fb in &ctx.framebuffers { d.destroy_framebuffer(fb, None); }
    ctx.framebuffers.clear();
    for &v in &ctx.views { d.destroy_image_view(v, None); }
    ctx.views.clear();
    d.destroy_image_view(ctx.depth_view, None);
    d.destroy_image(ctx.depth_img, None);
    d.free_memory(ctx.depth_mem, None);
    ctx.depth_view = vk::ImageView::null();
    ctx.depth_img = vk::Image::null();
    ctx.depth_mem = vk::DeviceMemory::null();
    ctx.swapchain_loader.destroy_swapchain(ctx.swapchain, None);
}

unsafe fn create_swapchain_related(ctx: &mut VkCtx, width: u32, height: u32) -> Result<(), RhiError> {
    let inst = &ctx.instance; let phys = ctx.phys; let d = &ctx.device; let surf = ctx.surface; let surf_loader = &ctx.surface_loader;
    let caps = surf_loader.get_physical_device_surface_capabilities(phys, surf).map_err(|_| RhiError::Failed)?;
    let formats = surf_loader.get_physical_device_surface_formats(phys, surf).map_err(|_| RhiError::Failed)?;
    let format = formats.iter().find(|f| f.format == vk::Format::B8G8R8A8_SRGB).cloned().unwrap_or(vk::SurfaceFormatKHR { format: vk::Format::B8G8R8A8_UNORM, color_space: vk::ColorSpaceKHR::SRGB_NONLINEAR }).format;
    let extent = if caps.current_extent.width != u32::MAX { caps.current_extent } else { vk::Extent2D { width, height } };
    let image_count = (caps.min_image_count + 1).min(if caps.max_image_count>0 { caps.max_image_count } else { u32::MAX });
    let scci = vk::SwapchainCreateInfoKHR::default().surface(surf).min_image_count(image_count).image_format(format).image_color_space(vk::ColorSpaceKHR::SRGB_NONLINEAR).image_extent(extent).image_array_layers(1).image_usage(vk::ImageUsageFlags::COLOR_ATTACHMENT | vk::ImageUsageFlags::TRANSFER_DST).image_sharing_mode(vk::SharingMode::EXCLUSIVE).pre_transform(caps.current_transform).composite_alpha(vk::CompositeAlphaFlagsKHR::OPAQUE).present_mode(vk::PresentModeKHR::FIFO).clipped(true);
    ctx.swapchain = ctx.swapchain_loader.create_swapchain(&scci, None).map_err(|_| RhiError::Failed)?;
    ctx.extent = extent;
    ctx.format = format;
    ctx.images = ctx.swapchain_loader.get_swapchain_images(ctx.swapchain).map_err(|_| RhiError::Failed)?;
    ctx.views = ctx.images.iter().map(|&img| {
        let ici = vk::ImageViewCreateInfo::default().image(img).view_type(vk::ImageViewType::TYPE_2D).format(format).subresource_range(vk::ImageSubresourceRange { aspect_mask: vk::ImageAspectFlags::COLOR, base_mip_level: 0, level_count: 1, base_array_layer: 0, layer_count: 1 });
        d.create_image_view(&ici, None).unwrap()
    }).collect();
    let depth_format = vk::Format::D32_SFLOAT;
    let depth_ci = vk::ImageCreateInfo::default().image_type(vk::ImageType::TYPE_2D).format(depth_format).extent(vk::Extent3D { width: extent.width, height: extent.height, depth: 1 }).mip_levels(1).array_layers(1).samples(vk::SampleCountFlags::TYPE_1).tiling(vk::ImageTiling::OPTIMAL).usage(vk::ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT).sharing_mode(vk::SharingMode::EXCLUSIVE).initial_layout(vk::ImageLayout::UNDEFINED);
    ctx.depth_img = d.create_image(&depth_ci, None).map_err(|_| RhiError::Failed)?;
    let dreq = d.get_image_memory_requirements(ctx.depth_img);
    let pmem2 = inst.get_physical_device_memory_properties(phys);
    let mtype_d = find_mem_type(pmem2, dreq, vk::MemoryPropertyFlags::DEVICE_LOCAL).ok_or(RhiError::Failed)?;
    ctx.depth_mem = d.allocate_memory(&vk::MemoryAllocateInfo::default().allocation_size(dreq.size).memory_type_index(mtype_d), None).map_err(|_| RhiError::Failed)?;
    d.bind_image_memory(ctx.depth_img, ctx.depth_mem, 0).map_err(|_| RhiError::Failed)?;
    let dview_ci = vk::ImageViewCreateInfo::default().image(ctx.depth_img).view_type(vk::ImageViewType::TYPE_2D).format(depth_format).subresource_range(vk::ImageSubresourceRange { aspect_mask: vk::ImageAspectFlags::DEPTH, base_mip_level: 0, level_count: 1, base_array_layer: 0, layer_count: 1 });
    ctx.depth_view = d.create_image_view(&dview_ci, None).map_err(|_| RhiError::Failed)?;
    ctx.framebuffers = ctx.views.iter().map(|&v| {
        let binding = [v, ctx.depth_view];
        let fci = vk::FramebufferCreateInfo::default().render_pass(ctx.renderpass).attachments(&binding).width(extent.width).height(extent.height).layers(1);
        d.create_framebuffer(&fci, None).unwrap()
    }).collect();
    let new_cpu_size = (extent.width as u64) * (extent.height as u64) * 4;
    let old_req = d.get_buffer_memory_requirements(ctx.cpu_buf);
    if old_req.size < new_cpu_size {
        d.destroy_buffer(ctx.cpu_buf, None);
        d.free_memory(ctx.cpu_mem, None);
        let cpu_ci = vk::BufferCreateInfo::default().size(new_cpu_size).usage(vk::BufferUsageFlags::TRANSFER_SRC).sharing_mode(vk::SharingMode::EXCLUSIVE);
        ctx.cpu_buf = d.create_buffer(&cpu_ci, None).map_err(|_| RhiError::Failed)?;
        let req = d.get_buffer_memory_requirements(ctx.cpu_buf);
        let pmem = inst.get_physical_device_memory_properties(phys);
        let mtype = find_mem_type(pmem, req, vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT).ok_or(RhiError::Failed)?;
        ctx.cpu_mem = d.allocate_memory(&vk::MemoryAllocateInfo::default().allocation_size(req.size).memory_type_index(mtype), None).map_err(|_| RhiError::Failed)?;
        d.bind_buffer_memory(ctx.cpu_buf, ctx.cpu_mem, 0).map_err(|_| RhiError::Failed)?;
    }
    Ok(())
}

unsafe fn recreate_pipelines(ctx: &mut VkCtx) -> Result<(), RhiError> {
    let d = &ctx.device;
    if ctx.pipeline_color_left != vk::Pipeline::null() { d.destroy_pipeline(ctx.pipeline_color_left, None); }
    if ctx.pipeline_color_right != vk::Pipeline::null() { d.destroy_pipeline(ctx.pipeline_color_right, None); }
    if ctx.pipeline_depth != vk::Pipeline::null() { d.destroy_pipeline(ctx.pipeline_depth, None); }
    if ctx.pipeline_layout != vk::PipelineLayout::null() { d.destroy_pipeline_layout(ctx.pipeline_layout, None); }
    let plci = vk::PipelineLayoutCreateInfo::default();
    ctx.pipeline_layout = d.create_pipeline_layout(&plci, None).map_err(|_| RhiError::Failed)?;
    let vsrc = "#version 450\nlayout(location=0) in vec4 in_pos;\nvoid main(){ gl_Position = in_pos; }";
    let fsrc_left = "#version 450\nlayout(location=0) out vec4 out_color;\nvoid main(){ out_color = vec4(0.25,0.75,0.25,1.0); }";
    let fsrc_right = "#version 450\nlayout(location=0) out vec4 out_color;\nvoid main(){ out_color = vec4(0.25,0.25,0.75,1.0); }";
    let options_v = glsl::Options::from(naga::ShaderStage::Vertex);
    let mut module_v: Module = glsl::Frontend::default().parse(&options_v, vsrc).map_err(|_| RhiError::Failed)?;
    let info_v = naga::valid::Validator::new(ValidationFlags::all(), Capabilities::all()).validate(&module_v).map_err(|_| RhiError::Failed)?;
    let mut spv_writer_v = spv::Writer::new(&spv::Options::default()).map_err(|_| RhiError::Failed)?;
    let mut vwords: Vec<u32> = Vec::new();
    spv_writer_v.write(&module_v, &info_v, None, &None, &mut vwords).map_err(|_| RhiError::Failed)?;
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
    let vs = ctx.device.create_shader_module(&vk::ShaderModuleCreateInfo::default().code(vwords.as_slice()), None).map_err(|_| RhiError::Failed)?;
    let fs_left = ctx.device.create_shader_module(&vk::ShaderModuleCreateInfo::default().code(fwords_left.as_slice()), None).map_err(|_| RhiError::Failed)?;
    let fs_right = ctx.device.create_shader_module(&vk::ShaderModuleCreateInfo::default().code(fwords_right.as_slice()), None).map_err(|_| RhiError::Failed)?;
    let entry_name = cstr("main"); let entry_cstr = entry_name.as_c_str();
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
    let binds = [bind]; let attrs = [attr];
    let vi = vk::PipelineVertexInputStateCreateInfo::default().vertex_binding_descriptions(&binds).vertex_attribute_descriptions(&attrs);
    let ia = vk::PipelineInputAssemblyStateCreateInfo::default().topology(vk::PrimitiveTopology::TRIANGLE_LIST);
    let vp = vk::Viewport::default().x(0.0).y(0.0).width(ctx.extent.width as f32).height(ctx.extent.height as f32).min_depth(0.0).max_depth(1.0);
    let sc = vk::Rect2D::default().offset(vk::Offset2D { x: 0, y: 0 }).extent(ctx.extent);
    let vps_viewports = [vp]; let vps_scissors = [sc];
    let vps = vk::PipelineViewportStateCreateInfo::default().viewports(&vps_viewports).scissors(&vps_scissors);
    let rs = vk::PipelineRasterizationStateCreateInfo::default().polygon_mode(vk::PolygonMode::FILL).cull_mode(vk::CullModeFlags::NONE).front_face(vk::FrontFace::COUNTER_CLOCKWISE).line_width(1.0);
    let ms = vk::PipelineMultisampleStateCreateInfo::default().rasterization_samples(vk::SampleCountFlags::TYPE_1);
    let ds = vk::PipelineDepthStencilStateCreateInfo::default().depth_test_enable(true).depth_write_enable(true).depth_compare_op(vk::CompareOp::LESS_OR_EQUAL);
    let blend_off = vk::PipelineColorBlendAttachmentState::default().blend_enable(false).color_write_mask(vk::ColorComponentFlags::R | vk::ColorComponentFlags::G | vk::ColorComponentFlags::B | vk::ColorComponentFlags::A);
    let blend_none = vk::PipelineColorBlendAttachmentState::default().blend_enable(false).color_write_mask(vk::ColorComponentFlags::empty());
    let cb_on_atts = [blend_off]; let cb_off_atts = [blend_none];
    let cb_on = vk::PipelineColorBlendStateCreateInfo::default().attachments(&cb_on_atts);
    let cb_off = vk::PipelineColorBlendStateCreateInfo::default().attachments(&cb_off_atts);
    let pci_color_left = vk::GraphicsPipelineCreateInfo::default().stages(&stages_left).vertex_input_state(&vi).input_assembly_state(&ia).viewport_state(&vps).rasterization_state(&rs).multisample_state(&ms).depth_stencil_state(&ds).color_blend_state(&cb_on).layout(ctx.pipeline_layout).render_pass(ctx.renderpass).subpass(0);
    let pci_color_right = vk::GraphicsPipelineCreateInfo::default().stages(&stages_right).vertex_input_state(&vi).input_assembly_state(&ia).viewport_state(&vps).rasterization_state(&rs).multisample_state(&ms).depth_stencil_state(&ds).color_blend_state(&cb_on).layout(ctx.pipeline_layout).render_pass(ctx.renderpass).subpass(0);
    let pci_depth = vk::GraphicsPipelineCreateInfo::default().stages(&stages_left).vertex_input_state(&vi).input_assembly_state(&ia).viewport_state(&vps).rasterization_state(&rs).multisample_state(&ms).depth_stencil_state(&ds).color_blend_state(&cb_off).layout(ctx.pipeline_layout).render_pass(ctx.renderpass).subpass(0);
    let pipelines = ctx.device.create_graphics_pipelines(vk::PipelineCache::null(), &[pci_color_left, pci_color_right, pci_depth], None).map_err(|_| RhiError::Failed)?;
    ctx.pipeline_color_left = pipelines[0]; ctx.pipeline_color_right = pipelines[1]; ctx.pipeline_depth = pipelines[2];
    ctx.device.destroy_shader_module(vs, None);
    ctx.device.destroy_shader_module(fs_left, None);
    ctx.device.destroy_shader_module(fs_right, None);
    Ok(())
}

pub fn vk_ctx_recreate(ctx: &mut VkCtx, width: u32, height: u32) -> Result<(), RhiError> {
    unsafe {
        let _ = ctx.device.device_wait_idle();
        destroy_swapchain_related(ctx);
        create_swapchain_related(ctx, width, height)?;
        recreate_pipelines(ctx)?;
        Ok(())
    }
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
        let ds = vk::PipelineDepthStencilStateCreateInfo::default().depth_test_enable(true).depth_write_enable(true).depth_compare_op(vk::CompareOp::LESS_OR_EQUAL);
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
