module;
export module Engine.Renderer:RHI.API;

import Language;
import :RHI.Types;

export namespace Engine::Renderer::RHI {
    export Language::StatusResult<Device> CreateDevice(const DeviceCreateInfo& ci) noexcept;
    export Language::StatusResult<Swapchain> CreateSwapchain(const Device& dev, const SurfaceInfo& si) noexcept;
    export Language::StatusResult<Queue> GetQueue(const Device& dev, QueueType type) noexcept;
    export Language::Status Submit(const Queue& q, CommandList& cmd, Fence* fence, const Swapchain& sc) noexcept;
    export void DestroySwapchain(const Device& dev, Swapchain& sc) noexcept;
    export void DestroyDevice(Device& dev) noexcept;
}