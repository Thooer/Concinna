module;
#include <vulkan/vulkan.h>

module Engine.Renderer;

import Lang;
import :RHI.Types;
import :RHI.API;
import Prm.WSI;
import Prm.Window;

namespace Engine::Renderer::RHI {
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
        Language::USize imageCount{0};
        VkImage* images{nullptr};
    };

    void CommandList::Begin() noexcept { m_recording = true; }
    void CommandList::End() noexcept { m_recording = false; }
    void CommandList::ClearColor(float r, float g, float b, float a) noexcept { m_clear[0]=r; m_clear[1]=g; m_clear[2]=b; m_clear[3]=a; }
    void CommandList::DrawTriangle2D(float,float,float,float,float,float,float,float,float,float) noexcept {}

    static Language::StatusResult<uint32_t> FindGraphicsQueueFamily(VkPhysicalDevice phys, VkSurfaceKHR surface) noexcept {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phys, &count, nullptr);
        if (count == 0) return Language::StatusResult<uint32_t>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed));
        VkQueueFamilyProperties* props = new VkQueueFamilyProperties[count];
        vkGetPhysicalDeviceQueueFamilyProperties(phys, &count, props);
        uint32_t idx = UINT32_MAX;
        for (uint32_t i = 0; i < count; ++i) {
            VkBool32 present = VK_FALSE;
            if (surface) { (void)vkGetPhysicalDeviceSurfaceSupportKHR(phys, i, surface, &present); }
            if ((props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) { idx = i; break; }
        }
        delete[] props;
        if (idx == UINT32_MAX) return Language::StatusResult<uint32_t>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed));
        return Language::StatusResult<uint32_t>::Ok(idx);
    }

    Language::StatusResult<Device> CreateDevice(const DeviceCreateInfo& ci) noexcept {
        VkApplicationInfo app{}; app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; app.pApplicationName = "NovaEngine"; app.applicationVersion = VK_MAKE_VERSION(1,0,0); app.pEngineName = "Nova"; app.engineVersion = VK_MAKE_VERSION(1,0,0); app.apiVersion = VK_API_VERSION_1_2;
        const char* exts[2] = { "VK_KHR_surface", "VK_KHR_win32_surface" };
        VkInstanceCreateInfo ici{}; ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; ici.pApplicationInfo = &app; ici.enabledExtensionCount = 2; ici.ppEnabledExtensionNames = exts;
        VkInstance instance{};
        if (vkCreateInstance(&ici, nullptr, &instance) != VK_SUCCESS) return Language::StatusResult<Device>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed));

        uint32_t pcount = 0; vkEnumeratePhysicalDevices(instance, &pcount, nullptr); if (pcount == 0) { vkDestroyInstance(instance, nullptr); return Language::StatusResult<Device>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed)); }
        VkPhysicalDevice phys{}; vkEnumeratePhysicalDevices(instance, &pcount, &phys);

        float prio = 1.0f;
        VkDeviceQueueCreateInfo qci{}; qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; qci.queueCount = 1; qci.pQueuePriorities = &prio; qci.queueFamilyIndex = 0;
        VkDeviceCreateInfo dci{}; dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; dci.queueCreateInfoCount = 1; dci.pQueueCreateInfos = &qci;
        VkDevice device{};
        if (vkCreateDevice(phys, &dci, nullptr, &device) != VK_SUCCESS) { vkDestroyInstance(instance, nullptr); return Language::StatusResult<Device>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed)); }

        VkQueue q{}; vkGetDeviceQueue(device, qci.queueFamilyIndex, 0, &q);
        VkCommandPoolCreateInfo cpci{}; cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO; cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; cpci.queueFamilyIndex = qci.queueFamilyIndex;
        VkCommandPool pool{}; if (vkCreateCommandPool(device, &cpci, nullptr, &pool) != VK_SUCCESS) { vkDestroyDevice(device, nullptr); vkDestroyInstance(instance, nullptr); return Language::StatusResult<Device>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed)); }

        VkDeviceState* state = new VkDeviceState{}; state->instance = instance; state->phys = phys; state->device = device; state->gfxFamily = qci.queueFamilyIndex; state->gfxQueue = q; state->cmdPool = pool;
        Device out{}; out.p = state; return Language::StatusResult<Device>::Ok(out);
    }

    Language::StatusResult<Swapchain> CreateSwapchain(const Device& dev, const SurfaceInfo& si) noexcept {
        if (!dev.p || !si.hwnd) return Language::StatusResult<Swapchain>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::InvalidArgument));
        VkDeviceState* ds = reinterpret_cast<VkDeviceState*>(dev.p);
        auto srSurf = Prm::CreateVulkanSurface(reinterpret_cast<void*>(ds->instance), Prm::WindowHandle{ si.hwnd });
        if (!srSurf.IsOk()) return Language::StatusResult<Swapchain>::Err(srSurf.ErrStatus());
        VkSurfaceKHR surface = reinterpret_cast<VkSurfaceKHR>(srSurf.OkValue());

        auto qf = FindGraphicsQueueFamily(ds->phys, surface); if (!qf.IsOk()) { vkDestroySurfaceKHR(ds->instance, surface, nullptr); return Language::StatusResult<Swapchain>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed)); }
        ds->gfxFamily = qf.OkValue(); vkGetDeviceQueue(ds->device, ds->gfxFamily, 0, &ds->gfxQueue);

        VkSurfaceCapabilitiesKHR caps{}; (void)vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ds->phys, surface, &caps);
        VkSurfaceFormatKHR fmt{}; fmt.format = VK_FORMAT_B8G8R8A8_UNORM; fmt.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkPresentModeKHR pm = VK_PRESENT_MODE_FIFO_KHR;
        VkExtent2D extent{}; extent.width = si.width ? si.width : ((caps.currentExtent.width == UINT32_MAX)?1280:caps.currentExtent.width); extent.height = si.height ? si.height : ((caps.currentExtent.height == UINT32_MAX)?720:caps.currentExtent.height);
        uint32_t minImg = caps.minImageCount + 1; if (caps.maxImageCount > 0 && minImg > caps.maxImageCount) minImg = caps.maxImageCount;

        VkSwapchainCreateInfoKHR sci2{}; sci2.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR; sci2.surface = surface; sci2.minImageCount = minImg; sci2.imageFormat = fmt.format; sci2.imageColorSpace = fmt.colorSpace; sci2.imageExtent = extent; sci2.imageArrayLayers = 1; sci2.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT; sci2.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; sci2.preTransform = caps.currentTransform; sci2.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; sci2.presentMode = pm; sci2.clipped = VK_TRUE;
        VkSwapchainKHR sc{}; if (vkCreateSwapchainKHR(ds->device, &sci2, nullptr, &sc) != VK_SUCCESS) { vkDestroySurfaceKHR(ds->instance, surface, nullptr); return Language::StatusResult<Swapchain>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed)); }

        uint32_t ic = 0; (void)vkGetSwapchainImagesKHR(ds->device, sc, &ic, nullptr);
        VkImage* imgs = nullptr; if (ic > 0) { imgs = new VkImage[ic]; (void)vkGetSwapchainImagesKHR(ds->device, sc, &ic, imgs); }

        VkSwapchainState* ss = new VkSwapchainState{}; ss->surface = surface; ss->swapchain = sc; ss->format = fmt.format; ss->extent = extent; ss->imageCount = static_cast<Language::USize>(ic); ss->images = imgs;
        Swapchain out{}; out.p = ss; return Language::StatusResult<Swapchain>::Ok(out);
    }

    Language::StatusResult<Queue> GetQueue(const Device& dev, QueueType type) noexcept {
        if (!dev.p) return Language::StatusResult<Queue>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::InvalidArgument));
        VkDeviceState* ds = reinterpret_cast<VkDeviceState*>(dev.p);
        VkQueue q = ds->gfxQueue;
        (void)type;
        struct VkQueueState { VkQueue q; VkDeviceState* ds; };
        VkQueueState* qs = new VkQueueState{ q, ds };
        Queue out{}; out.p = qs; return Language::StatusResult<Queue>::Ok(out);
    }

    Language::Status Submit(const Queue& q, CommandList& cmd, Fence* fence, const Swapchain& sc) noexcept {
        if (!q.p || !sc.p) return Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::InvalidArgument);
        struct VkQueueState { VkQueue q; VkDeviceState* ds; };
        VkQueueState* qs = reinterpret_cast<VkQueueState*>(q.p);
        VkSwapchainState* ss = reinterpret_cast<VkSwapchainState*>(sc.p);
        VkDeviceState* ds = qs->ds;
        (void)fence;

        VkSemaphoreCreateInfo semInfo{}; semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkSemaphore imageAvailable{}; if (vkCreateSemaphore(ds->device, &semInfo, nullptr, &imageAvailable) != VK_SUCCESS) return Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed);
        VkSemaphore renderFinished{}; if (vkCreateSemaphore(ds->device, &semInfo, nullptr, &renderFinished) != VK_SUCCESS) { vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed); }

        uint32_t imageIndex = 0;
        if (vkAcquireNextImageKHR(ds->device, ss->swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex) != VK_SUCCESS) { vkDestroySemaphore(ds->device, renderFinished, nullptr); vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed); }

        VkCommandBufferAllocateInfo ai{}; ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; ai.commandPool = ds->cmdPool; ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandBufferCount = 1;
        VkCommandBuffer cmdBuf{}; if (vkAllocateCommandBuffers(ds->device, &ai, &cmdBuf) != VK_SUCCESS) { vkDestroySemaphore(ds->device, renderFinished, nullptr); vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed); }

        VkCommandBufferBeginInfo bi{}; bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        (void)vkBeginCommandBuffer(cmdBuf, &bi);

        VkImageMemoryBarrier toClear{}; toClear.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; toClear.srcAccessMask = 0; toClear.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; toClear.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; toClear.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; toClear.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; toClear.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; toClear.image = ss->images[imageIndex]; toClear.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; toClear.subresourceRange.baseMipLevel = 0; toClear.subresourceRange.levelCount = 1; toClear.subresourceRange.baseArrayLayer = 0; toClear.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &toClear);

        VkClearColorValue clr{}; const float* c = cmd.ClearColorRGBA(); clr.float32[0] = c[0]; clr.float32[1] = c[1]; clr.float32[2] = c[2]; clr.float32[3] = c[3];
        VkImageSubresourceRange range{}; range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; range.baseMipLevel = 0; range.levelCount = 1; range.baseArrayLayer = 0; range.layerCount = 1;
        vkCmdClearColorImage(cmdBuf, ss->images[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clr, 1, &range);

        VkImageMemoryBarrier toPresent{}; toPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; toPresent.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; toPresent.dstAccessMask = 0; toPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; toPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; toPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; toPresent.image = ss->images[imageIndex]; toPresent.subresourceRange = range;
        vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &toPresent);

        (void)vkEndCommandBuffer(cmdBuf);

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        VkSubmitInfo si{}; si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; si.waitSemaphoreCount = 1; si.pWaitSemaphores = &imageAvailable; si.pWaitDstStageMask = &waitStage; si.commandBufferCount = 1; si.pCommandBuffers = &cmdBuf; si.signalSemaphoreCount = 1; si.pSignalSemaphores = &renderFinished;
        if (vkQueueSubmit(qs->q, 1, &si, VK_NULL_HANDLE) != VK_SUCCESS) { vkFreeCommandBuffers(ds->device, ds->cmdPool, 1, &cmdBuf); vkDestroySemaphore(ds->device, renderFinished, nullptr); vkDestroySemaphore(ds->device, imageAvailable, nullptr); return Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed); }

        VkPresentInfoKHR pi{}; pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; pi.waitSemaphoreCount = 1; pi.pWaitSemaphores = &renderFinished; pi.swapchainCount = 1; pi.pSwapchains = &ss->swapchain; pi.pImageIndices = &imageIndex;
        (void)vkQueuePresentKHR(qs->q, &pi);

        (void)vkQueueWaitIdle(qs->q);
        vkFreeCommandBuffers(ds->device, ds->cmdPool, 1, &cmdBuf);
        vkDestroySemaphore(ds->device, renderFinished, nullptr);
        vkDestroySemaphore(ds->device, imageAvailable, nullptr);
        return Language::Ok(Language::StatusDomain::Platform());
    }

    void DestroySwapchain(const Device& dev, Swapchain& sc) noexcept {
        if (!dev.p || !sc.p) return;
        VkDeviceState* ds = reinterpret_cast<VkDeviceState*>(dev.p);
        VkSwapchainState* ss = reinterpret_cast<VkSwapchainState*>(sc.p);
        if (ss->images) { delete[] ss->images; ss->images = nullptr; ss->imageCount = 0; }
        if (ss->swapchain) vkDestroySwapchainKHR(ds->device, ss->swapchain, nullptr);
        if (ss->surface) (void)Platform::WSI::DestroyVulkanSurface(reinterpret_cast<void*>(ds->instance), reinterpret_cast<void*>(ss->surface));
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
