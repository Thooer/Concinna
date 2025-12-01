module;
#include <vulkan/vulkan.h>

module Sys.RHI:Backend;

import Lang;
import :Types;
import :API;
import Prm.WSI;
import Prm.Window;
import Profiling;

namespace Sys {
    struct VkDeviceState {
        VkInstance instance{};
        VkPhysicalDevice phys{};
        VkDevice device{};
        uint32_t gfxFamily{0};
        VkQueue gfxQueue{};
        VkCommandPool cmdPool{};
    };

    struct VkSwapchainState {
        VkSurfaceKHR surface{};
        VkSwapchainKHR swapchain{};
        VkFormat format{VK_FORMAT_B8G8R8A8_UNORM};
        VkExtent2D extent{1280,720};
        USize imageCount{0};
        VkImage* images{nullptr};
    };

    void CommandList::Begin() noexcept { m_recording = true; m_triCount = 0; }
    void CommandList::End() noexcept { m_recording = false; }
    void CommandList::ClearColor(float r, float g, float b, float a) noexcept { m_clear[0]=r; m_clear[1]=g; m_clear[2]=b; m_clear[3]=a; }
    void CommandList::DrawTriangle2D(float x0,float y0,float x1,float y1,float x2,float y2,float r,float g,float b,float a) noexcept {
        if (!m_recording) return;
        if (m_triCount < static_cast<USize>(16)) {
            auto& t = m_tris[static_cast<size_t>(m_triCount++)];
            t.x0=x0; t.y0=y0; t.x1=x1; t.y1=y1; t.x2=x2; t.y2=y2; t.c0=r; t.c1=g; t.c2=b; t.c3=a;
        }
    }

    static StatusResult<uint32_t> FindGraphicsQueueFamily(VkPhysicalDevice phys, VkSurfaceKHR surface) noexcept {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phys, &count, nullptr);
        if (count == 0) return StatusResult<uint32_t>::Err(Err(StatusDomain::Platform(), StatusCode::Failed));
        VkQueueFamilyProperties* props = new VkQueueFamilyProperties[count];
        vkGetPhysicalDeviceQueueFamilyProperties(phys, &count, props);
        uint32_t idx = UINT32_MAX;
        for (uint32_t i = 0; i < count; ++i) {
            VkBool32 present = VK_FALSE;
            if (surface) { (void)vkGetPhysicalDeviceSurfaceSupportKHR(phys, i, surface, &present); }
            if ((props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) { idx = i; break; }
        }
        delete[] props;
        if (idx == UINT32_MAX) return StatusResult<uint32_t>::Err(Err(StatusDomain::Platform(), StatusCode::Failed));
        return StatusResult<uint32_t>::Ok(idx);
    }

    static StatusResult<uint32_t> FindAnyGraphicsQueueFamily(VkPhysicalDevice phys) noexcept {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phys, &count, nullptr);
        if (count == 0) return StatusResult<uint32_t>::Err(Err(StatusDomain::Platform(), StatusCode::Failed));
        VkQueueFamilyProperties* props = new VkQueueFamilyProperties[count];
        vkGetPhysicalDeviceQueueFamilyProperties(phys, &count, props);
        uint32_t idx = UINT32_MAX;
        for (uint32_t i = 0; i < count; ++i) {
            if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { idx = i; break; }
        }
        delete[] props;
        if (idx == UINT32_MAX) return StatusResult<uint32_t>::Err(Err(StatusDomain::Platform(), StatusCode::Failed));
        return StatusResult<uint32_t>::Ok(idx);
    }

    StatusResult<Device> CreateDevice(const DeviceCreateInfo& ci) noexcept {
        VkApplicationInfo app{}; app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; app.pApplicationName = "NovaEngine"; app.applicationVersion = VK_MAKE_VERSION(1,0,0); app.pEngineName = "Nova"; app.engineVersion = VK_MAKE_VERSION(1,0,0); app.apiVersion = VK_API_VERSION_1_2;
        const char* exts[2] = { "VK_KHR_surface", "VK_KHR_win32_surface" };
        VkInstanceCreateInfo ici{}; ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; ici.pApplicationInfo = &app; ici.enabledExtensionCount = 2; ici.ppEnabledExtensionNames = exts;
        VkInstance instance{};
        if (vkCreateInstance(&ici, nullptr, &instance) != VK_SUCCESS) return StatusResult<Device>::Err(Err(StatusDomain::Platform(), StatusCode::Failed));

        uint32_t pcount = 0; vkEnumeratePhysicalDevices(instance, &pcount, nullptr); if (pcount == 0) { vkDestroyInstance(instance, nullptr); return StatusResult<Device>::Err(Err(StatusDomain::Platform(), StatusCode::Failed)); }
        VkPhysicalDevice phys{}; vkEnumeratePhysicalDevices(instance, &pcount, &phys);

        auto gfxFam = FindAnyGraphicsQueueFamily(phys); if (!gfxFam.IsOk()) { vkDestroyInstance(instance, nullptr); return StatusResult<Device>::Err(Err(StatusDomain::Platform(), StatusCode::Failed)); }
        float prio = 1.0f;
        VkDeviceQueueCreateInfo qci{}; qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; qci.queueCount = 1; qci.pQueuePriorities = &prio; qci.queueFamilyIndex = gfxFam.OkValue();
        VkDeviceCreateInfo dci{}; dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; dci.queueCreateInfoCount = 1; dci.pQueueCreateInfos = &qci;
        VkDevice device{};
        if (vkCreateDevice(phys, &dci, nullptr, &device) != VK_SUCCESS) { vkDestroyInstance(instance, nullptr); return StatusResult<Device>::Err(Err(StatusDomain::Platform(), StatusCode::Failed)); }

        VkQueue q{}; vkGetDeviceQueue(device, qci.queueFamilyIndex, 0, &q);
        VkCommandPoolCreateInfo cpci{}; cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; cpci.queueFamilyIndex = qci.queueFamilyIndex;
        VkCommandPool pool{}; if (vkCreateCommandPool(device, &cpci, nullptr, &pool) != VK_SUCCESS) { vkDestroyDevice(device, nullptr); vkDestroyInstance(instance, nullptr); return StatusResult<Device>::Err(Err(StatusDomain::Platform(), StatusCode::Failed)); }

        VkDeviceState* state = new VkDeviceState{}; state->instance = instance; state->phys = phys; state->device = device; state->gfxFamily = qci.queueFamilyIndex; state->gfxQueue = q; state->cmdPool = pool;
        Device out{}; out.p = state; return StatusResult<Device>::Ok(out);
    }

    StatusResult<Swapchain> CreateSwapchain(const Device& dev, const SurfaceInfo& si) noexcept {
        if (!dev.p || !si.hwnd) return StatusResult<Swapchain>::Err(Err(StatusDomain::Platform(), StatusCode::InvalidArgument));
        VkDeviceState* ds = reinterpret_cast<VkDeviceState*>(dev.p);
        auto srSurf = Prm::CreateVulkanSurface(reinterpret_cast<void*>(ds->instance), Prm::WindowHandle{ si.hwnd });
        if (!srSurf.IsOk()) return StatusResult<Swapchain>::Err(srSurf.ErrStatus());
        VkSurfaceKHR surface = reinterpret_cast<VkSurfaceKHR>(srSurf.OkValue());

        auto qf = FindGraphicsQueueFamily(ds->phys, surface); if (!qf.IsOk()) { vkDestroySurfaceKHR(ds->instance, surface, nullptr); return StatusResult<Swapchain>::Err(Err(StatusDomain::Platform(), StatusCode::Failed)); }
        ds->gfxFamily = qf.OkValue(); vkGetDeviceQueue(ds->device, ds->gfxFamily, 0, &ds->gfxQueue);

        VkSurfaceCapabilitiesKHR caps{}; (void)vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ds->phys, surface, &caps);
        VkSurfaceFormatKHR fmt{}; fmt.format = VK_FORMAT_B8G8R8A8_UNORM; fmt.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkPresentModeKHR pm = VK_PRESENT_MODE_FIFO_KHR;
        VkExtent2D extent{}; extent.width = si.width ? si.width : ((caps.currentExtent.width == UINT32_MAX)?1280:caps.currentExtent.width); extent.height = si.height ? si.height : ((caps.currentExtent.height == UINT32_MAX)?720:caps.currentExtent.height);
        uint32_t minImg = caps.minImageCount + 1; if (caps.maxImageCount > 0 && minImg > caps.maxImageCount) minImg = caps.maxImageCount;

        VkSwapchainCreateInfoKHR sci2{}; sci2.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR; sci2.surface = surface; sci2.minImageCount = minImg; sci2.imageFormat = fmt.format; sci2.imageColorSpace = fmt.colorSpace; sci2.imageExtent = extent; sci2.imageArrayLayers = 1; sci2.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; sci2.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; sci2.preTransform = caps.currentTransform; sci2.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; sci2.presentMode = pm; sci2.clipped = VK_TRUE;
        VkSwapchainKHR sc{}; if (vkCreateSwapchainKHR(ds->device, &sci2, nullptr, &sc) != VK_SUCCESS) { vkDestroySurfaceKHR(ds->instance, surface, nullptr); return StatusResult<Swapchain>::Err(Err(StatusDomain::Platform(), StatusCode::Failed)); }

        uint32_t ic = 0; (void)vkGetSwapchainImagesKHR(ds->device, sc, &ic, nullptr);
        VkImage* imgs = nullptr; if (ic > 0) { imgs = new VkImage[ic]; (void)vkGetSwapchainImagesKHR(ds->device, sc, &ic, imgs); }

        VkSwapchainState* ss = new VkSwapchainState{}; ss->surface = surface; ss->swapchain = sc; ss->format = fmt.format; ss->extent = extent; ss->imageCount = static_cast<USize>(ic); ss->images = imgs;
        Swapchain out{}; out.p = ss; return StatusResult<Swapchain>::Ok(out);
    }

    StatusResult<Queue> GetQueue(const Device& dev, QueueType type) noexcept {
        if (!dev.p) return StatusResult<Queue>::Err(Err(StatusDomain::Platform(), StatusCode::InvalidArgument));
        VkDeviceState* ds = reinterpret_cast<VkDeviceState*>(dev.p);
        VkQueue q = ds->gfxQueue;
        (void)type;
        struct VkQueueState { VkQueue q; VkDeviceState* ds; };
        VkQueueState* qs = new VkQueueState{ q, ds };
        Queue out{}; out.p = qs; return StatusResult<Queue>::Ok(out);
    }

    Status Submit(const Queue& q, CommandList& cmd, Fence* fence, const Swapchain& sc) noexcept {
        if (!q.p || !sc.p) return Err(StatusDomain::Platform(), StatusCode::InvalidArgument);
        struct VkQueueState { VkQueue q; VkDeviceState* ds; };
        VkQueueState* qs = reinterpret_cast<VkQueueState*>(q.p);
        VkSwapchainState* ss = reinterpret_cast<VkSwapchainState*>(sc.p);
        VkDeviceState* ds = qs->ds;
        (void)fence;
        Profiling::FrameMark();

        VkSemaphoreCreateInfo semInfo{}; semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkSemaphore imageAvailable{}; if (vkCreateSemaphore(ds->device, &semInfo, nullptr, &imageAvailable) != VK_SUCCESS) return Err(StatusDomain::Platform(), StatusCode::Failed);
        VkSemaphore renderFinished{}; if (vkCreateSemaphore(ds->device, &semInfo, nullptr, &renderFinished) != VK_SUCCESS) { vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Err(StatusDomain::Platform(), StatusCode::Failed); }

        uint32_t imageIndex = 0;
        if (vkAcquireNextImageKHR(ds->device, ss->swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS) { vkDestroySemaphore(ds->device, renderFinished, nullptr); vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Err(StatusDomain::Platform(), StatusCode::Failed); }

        VkCommandBufferAllocateInfo ai{}; ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; ai.commandPool = ds->cmdPool; ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandBufferCount = 1;
        VkCommandBuffer cmdBuf{}; if (vkAllocateCommandBuffers(ds->device, &ai, &cmdBuf) != VK_SUCCESS) { vkDestroySemaphore(ds->device, renderFinished, nullptr); vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Err(StatusDomain::Platform(), StatusCode::Failed); }

        VkCommandBufferBeginInfo bi{}; bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        (void)vkBeginCommandBuffer(cmdBuf, &bi);

        VkImageViewCreateInfo ivci{}; ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; ivci.image = ss->images[imageIndex]; ivci.viewType = VK_IMAGE_VIEW_TYPE_2D; ivci.format = ss->format; ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; ivci.subresourceRange.baseMipLevel = 0; ivci.subresourceRange.levelCount = 1; ivci.subresourceRange.baseArrayLayer = 0; ivci.subresourceRange.layerCount = 1;
        VkImageView imgView{}; if (vkCreateImageView(ds->device, &ivci, nullptr, &imgView) != VK_SUCCESS) { vkFreeCommandBuffers(ds->device, ds->cmdPool, 1, &cmdBuf); vkDestroySemaphore(ds->device, renderFinished, nullptr); vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Err(StatusDomain::Platform(), StatusCode::Failed); }

        VkAttachmentDescription color{}; color.format = ss->format; color.samples = VK_SAMPLE_COUNT_1_BIT; color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; color.storeOp = VK_ATTACHMENT_STORE_OP_STORE; color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colorRef{}; colorRef.attachment = 0; colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpass{}; subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; subpass.colorAttachmentCount = 1; subpass.pColorAttachments = &colorRef;
        VkSubpassDependency dep{}; dep.srcSubpass = VK_SUBPASS_EXTERNAL; dep.dstSubpass = 0; dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; dep.srcAccessMask = 0; dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        VkRenderPassCreateInfo rpci{}; rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO; rpci.attachmentCount = 1; rpci.pAttachments = &color; rpci.subpassCount = 1; rpci.pSubpasses = &subpass; rpci.dependencyCount = 1; rpci.pDependencies = &dep;
        VkRenderPass renderPass{}; if (vkCreateRenderPass(ds->device, &rpci, nullptr, &renderPass) != VK_SUCCESS) { vkDestroyImageView(ds->device, imgView, nullptr); vkFreeCommandBuffers(ds->device, ds->cmdPool, 1, &cmdBuf); vkDestroySemaphore(ds->device, renderFinished, nullptr); vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Err(StatusDomain::Platform(), StatusCode::Failed); }

        VkFramebufferCreateInfo fbci{}; fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO; fbci.renderPass = renderPass; fbci.attachmentCount = 1; fbci.pAttachments = &imgView; fbci.width = ss->extent.width; fbci.height = ss->extent.height; fbci.layers = 1;
        VkFramebuffer fb{}; if (vkCreateFramebuffer(ds->device, &fbci, nullptr, &fb) != VK_SUCCESS) { vkDestroyRenderPass(ds->device, renderPass, nullptr); vkDestroyImageView(ds->device, imgView, nullptr); vkFreeCommandBuffers(ds->device, ds->cmdPool, 1, &cmdBuf); vkDestroySemaphore(ds->device, renderFinished, nullptr); vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Err(StatusDomain::Platform(), StatusCode::Failed); }

        VkClearValue clear{}; const float* c = cmd.ClearColorRGBA(); clear.color.float32[0]=c[0]; clear.color.float32[1]=c[1]; clear.color.float32[2]=c[2]; clear.color.float32[3]=c[3];
        VkRenderPassBeginInfo rpb{}; rpb.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO; rpb.renderPass = renderPass; rpb.framebuffer = fb; rpb.renderArea.offset = {0,0}; rpb.renderArea.extent = ss->extent; rpb.clearValueCount = 1; rpb.pClearValues = &clear;
        vkCmdBeginRenderPass(cmdBuf, &rpb, VK_SUBPASS_CONTENTS_INLINE);
        bool drewTriangle = false;
        {
            static const uint32_t kVertSpv[] = {
                0x07230203,0x00010000,0x0008000a,0x00000013,
                0x00000000,0x00020011,0x00000001,0x0006000b,
                0x00000001,0x4c534c47,0x00000000,0x00000001,
                0x00000000,0x0003000e,0x00000000,0x00000001,
                0x0007000f,0x00000000,0x00000004,0x6d61696e,
                0x00000000,0x0000000b,0x0000000f,0x00030003,
                0x00000002,0x000001c8,0x00040005,0x00000004,
                0x6d61696e,0x00000000,0x00050005,0x0000000b,
                0x6f736974,0x6e6f6974,0x00000073,0x00040005,
                0x0000000f,0x565f4f47,0x00000000,0x00030047,
                0x0000000f,0x0000000e,0x00020013,0x00000002,
                0x00030021,0x00000003,0x00000002,0x00040015,
                0x00000006,0x00000020,0x00000000,0x0004002b,
                0x00000006,0x00000007,0x00000003,0x0004002b,
                0x00000006,0x00000008,0x00000000,0x0004002b,
                0x00000006,0x00000009,0x00000001,0x0004002b,
                0x00000006,0x0000000a,0x00000002,0x0004003b,
                0x00000006,0x0000000b,0x00000007,0x0004003b,
                0x00000006,0x0000000f,0x00000007,0x00020014,
                0x00000010,0x00040020,0x00000011,0x00000007,
                0x00000006,0x0004002b,0x00000006,0x00000012,
                0x00000000,0x00050036,0x00000002,0x00000004,
                0x00000000,0x00000003,0x000200f8,0x00000005,
                0x0003003e,0x0000000b,0x00000008,0x0003003e,
                0x0000000f,0x00000012,0x000100fd,0x00010038
            };
            static const uint32_t kFragSpv[] = {
                0x07230203,0x00010000,0x0008000a,0x0000000b,
                0x00000000,0x00020011,0x00000001,0x0006000b,
                0x00000001,0x4c534c47,0x00000000,0x00000001,
                0x00000000,0x0003000e,0x00000000,0x00000001,
                0x0007000f,0x00000005,0x00000004,0x6d61696e,
                0x00000000,0x00000009,0x0000000a,0x00030003,
                0x00000002,0x000001c8,0x00040005,0x00000004,
                0x6d61696e,0x00000000,0x00040005,0x00000009,
                0x6f436f6c,0x00000072,0x00030047,0x00000009,
                0x0000000e,0x00040047,0x0000000a,0x0000001e,
                0x00000000,0x00020013,0x00000002,0x00030021,
                0x00000003,0x00000002,0x00040015,0x00000006,
                0x00000020,0x00000000,0x0004002b,0x00000006,
                0x00000007,0x00000000,0x0004002b,0x00000006,
                0x00000008,0x00000000,0x0004003b,0x00000006,
                0x00000009,0x00000007,0x0004003b,0x00000006,
                0x0000000a,0x00000003,0x00050036,0x00000002,
                0x00000004,0x00000000,0x00000003,0x000200f8,
                0x00000005,0x0003003e,0x00000009,0x00000007,
                0x000100fd,0x00010038
            };

            VkShaderModule vs{}, fs{};
            VkShaderModuleCreateInfo smci{}; smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            smci.codeSize = sizeof(kVertSpv); smci.pCode = kVertSpv;
            if (vkCreateShaderModule(ds->device, &smci, nullptr, &vs) == VK_SUCCESS) {
                smci.codeSize = sizeof(kFragSpv); smci.pCode = kFragSpv;
                if (vkCreateShaderModule(ds->device, &smci, nullptr, &fs) == VK_SUCCESS) {
                    VkPipelineLayoutCreateInfo plci{}; plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                    VkPipelineLayout layout{}; if (vkCreatePipelineLayout(ds->device, &plci, nullptr, &layout) == VK_SUCCESS) {
                        VkPipelineShaderStageCreateInfo stages[2]{};
                        stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT; stages[0].module = vs; stages[0].pName = "main";
                        stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; stages[1].module = fs; stages[1].pName = "main";
                        VkPipelineVertexInputStateCreateInfo vi{}; vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                        VkPipelineInputAssemblyStateCreateInfo ia{}; ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                        VkViewport vp{}; vp.x = 0.0f; vp.y = 0.0f; vp.width = static_cast<float>(ss->extent.width); vp.height = static_cast<float>(ss->extent.height); vp.minDepth = 0.0f; vp.maxDepth = 1.0f;
                        VkRect2D sc{}; sc.offset = {0,0}; sc.extent = ss->extent;
                        VkPipelineViewportStateCreateInfo vps{}; vps.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO; vps.viewportCount = 1; vps.pViewports = &vp; vps.scissorCount = 1; vps.pScissors = &sc;
                        VkPipelineRasterizationStateCreateInfo rs{}; rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO; rs.polygonMode = VK_POLYGON_MODE_FILL; rs.cullMode = VK_CULL_MODE_NONE; rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; rs.lineWidth = 1.0f;
                        VkPipelineMultisampleStateCreateInfo ms{}; ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                        VkPipelineColorBlendAttachmentState cba{}; cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                        VkPipelineColorBlendStateCreateInfo cb{}; cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; cb.attachmentCount = 1; cb.pAttachments = &cba;
                        VkGraphicsPipelineCreateInfo gp{}; gp.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO; gp.stageCount = 2; gp.pStages = stages; gp.pVertexInputState = &vi; gp.pInputAssemblyState = &ia; gp.pViewportState = &vps; gp.pRasterizationState = &rs; gp.pMultisampleState = &ms; gp.pColorBlendState = &cb; gp.layout = layout; gp.renderPass = renderPass; gp.subpass = 0;
                        VkPipeline pipeline{};
                        if (vkCreateGraphicsPipelines(ds->device, VK_NULL_HANDLE, 1, &gp, nullptr, &pipeline) == VK_SUCCESS) {
                            vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                            vkCmdDraw(cmdBuf, 3, 1, 0, 0);
                            drewTriangle = true;
                            vkDestroyPipeline(ds->device, pipeline, nullptr);
                        }
                        vkDestroyPipelineLayout(ds->device, layout, nullptr);
                    }
                }
                vkDestroyShaderModule(ds->device, fs, nullptr);
            }
            vkDestroyShaderModule(ds->device, vs, nullptr);
        }
        vkCmdEndRenderPass(cmdBuf);

        (void)vkEndCommandBuffer(cmdBuf);

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkSubmitInfo si{}; si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; si.waitSemaphoreCount = 1; si.pWaitSemaphores = &imageAvailable; si.pWaitDstStageMask = &waitStage; si.commandBufferCount = 1; si.pCommandBuffers = &cmdBuf; si.signalSemaphoreCount = 1; si.pSignalSemaphores = &renderFinished;
        if (vkQueueSubmit(qs->q, 1, &si, VK_NULL_HANDLE) != VK_SUCCESS) { vkFreeCommandBuffers(ds->device, ds->cmdPool, 1, &cmdBuf); vkDestroySemaphore(ds->device, renderFinished, nullptr); vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Err(StatusDomain::Platform(), StatusCode::Failed); }

        VkPresentInfoKHR pi{}; pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; pi.waitSemaphoreCount = 1; pi.pWaitSemaphores = &renderFinished; pi.swapchainCount = 1; pi.pSwapchains = &ss->swapchain; pi.pImageIndices = &imageIndex;
        (void)vkQueuePresentKHR(qs->q, &pi);

        (void)vkQueueWaitIdle(qs->q);
        vkFreeCommandBuffers(ds->device, ds->cmdPool, 1, &cmdBuf);
        vkDestroyFramebuffer(ds->device, fb, nullptr);
        vkDestroyRenderPass(ds->device, renderPass, nullptr);
        vkDestroyImageView(ds->device, imgView, nullptr);
        vkDestroySemaphore(ds->device, renderFinished, nullptr);
        vkDestroySemaphore(ds->device, imageAvailable, nullptr);
        return Ok(StatusDomain::Platform());
    }

    void DestroySwapchain(const Device& dev, Swapchain& sc) noexcept {
        if (!dev.p || !sc.p) return;
        VkDeviceState* ds = reinterpret_cast<VkDeviceState*>(dev.p);
        VkSwapchainState* ss = reinterpret_cast<VkSwapchainState*>(sc.p);
        if (ss->images) { delete[] ss->images; ss->images = nullptr; ss->imageCount = 0; }
        if (ss->swapchain) vkDestroySwapchainKHR(ds->device, ss->swapchain, nullptr);
        if (ss->surface) (void)Prm::DestroyVulkanSurface(reinterpret_cast<void*>(ds->instance), reinterpret_cast<void*>(ss->surface));
        delete ss; sc.p = nullptr;
    }

    void DestroyDevice(Device& dev) noexcept {
        if (!dev.p) return;
        VkDeviceState* ds = reinterpret_cast<VkDeviceState*>(dev.p);
        if (ds->cmdPool) vkDestroyCommandPool(ds->device, ds->cmdPool, nullptr);
        if (ds->device) vkDestroyDevice(ds->device, nullptr);
        if (ds->instance) vkDestroyInstance(ds->instance, nullptr);
        delete ds; dev.p = nullptr;
    }
}
