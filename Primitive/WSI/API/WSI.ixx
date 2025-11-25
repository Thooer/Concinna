module;
export module Platform:WSI;

import Prm;
import :Window;

export namespace Platform::WSI {
    export Expect<void*> CreateVulkanSurface(void* instance, Platform::WindowHandle hwnd) noexcept;
    export Status DestroyVulkanSurface(void* instance, void* surface) noexcept;

    export Expect<void*> CreateCpuPresent(Platform::WindowHandle hwnd, UInt32 width, UInt32 height) noexcept;
    export Status DestroyCpuPresent(void* state) noexcept;
    export Expect<void*> CpuGetBuffer(void* state) noexcept;
    export UInt32 CpuGetPitch(void* state) noexcept;
    export Status CpuPresent(void* state) noexcept;
}