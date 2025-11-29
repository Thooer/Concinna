module;
export module Eng.Renderer:RHI.API;

import Lang;
import :RHI.Types;

export namespace Eng::Renderer::RHI {
    export Expect<Device> CreateDevice(const DeviceCreateInfo& ci) noexcept;
    export Expect<Swapchain> CreateSwapchain(const Device& dev, const SurfaceInfo& si) noexcept;
    export Expect<Queue> GetQueue(const Device& dev, QueueType type) noexcept;
    export Status Submit(const Queue& q, CommandList& cmd, Fence* fence, const Swapchain& sc) noexcept;
    export void DestroySwapchain(const Device& dev, Swapchain& sc) noexcept;
    export void DestroyDevice(Device& dev) noexcept;
}
