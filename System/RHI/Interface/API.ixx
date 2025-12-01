module;
export module Sys.RHI:API;

import Lang;
import :Types;
export import :Types;

export namespace Sys {
    export Expect<Device> CreateDevice() noexcept;
    export Expect<Device> CreateDevice(const DeviceCreateInfo& ci) noexcept;
    export Expect<Swapchain> CreateSwapchain(const Device& dev, const SurfaceInfo& si) noexcept;
    export Expect<Queue> GetQueue(const Device& dev, QueueType type) noexcept;
    export Status Submit(const Queue& q, CommandList& cmd, Fence* fence, const Swapchain& sc) noexcept;
    export void DestroySwapchain(const Device& dev, Swapchain& sc) noexcept;
    export void DestroyDevice(Device& dev) noexcept;
}
