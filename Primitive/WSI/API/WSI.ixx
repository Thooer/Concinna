module;
export module Prm.WSI;

import Prm.Window;

export namespace Prm {
    export Expect<void*> CreateVulkanSurface(void* instance, WindowHandle hwnd) noexcept;
    export Status DestroyVulkanSurface(void* instance, void* surface) noexcept;

    export Expect<void*> CreateCpuPresent(WindowHandle hwnd, UInt32 width, UInt32 height) noexcept;
    export Status DestroyCpuPresent(void* state) noexcept;
    export Expect<void*> CpuGetBuffer(void* state) noexcept;
    export UInt32 CpuGetPitch(void* state) noexcept;
    export Status CpuPresent(void* state) noexcept;
}
