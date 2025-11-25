module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifdef HAS_VULKAN
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#endif

module Prm.WSI;

import Prm.Window;
import :WSI;

namespace Prm {
#ifdef HAS_VULKAN
    Expect<void*> CreateVulkanSurface(void* instance, WindowHandle hwnd) noexcept {
        if (!instance || !hwnd.Get()) return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        VkInstance vi = reinterpret_cast<VkInstance>(instance);
        HWND h = reinterpret_cast<HWND>(hwnd.Get());
        VkWin32SurfaceCreateInfoKHR sci{}; sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR; sci.hinstance = GetModuleHandleA(nullptr); sci.hwnd = h;
        VkSurfaceKHR surface{};
        if (vkCreateWin32SurfaceKHR(vi, &sci, nullptr, &surface) != VK_SUCCESS) {
            return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        return Expect<void*>::Ok(reinterpret_cast<void*>(surface));
    }

    Status DestroyVulkanSurface(void* instance, void* surface) noexcept {
        if (!instance || !surface) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        vkDestroySurfaceKHR(reinterpret_cast<VkInstance>(instance), reinterpret_cast<VkSurfaceKHR>(surface), nullptr);
        return Ok(StatusDomain::System());
    }
#else
    Expect<void*> CreateVulkanSurface(void*, WindowHandle) noexcept {
        return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Status DestroyVulkanSurface(void*, void*) noexcept {
        return Err(StatusDomain::System(), StatusCode::Unsupported);
    }
#endif

    struct CpuPresentState {
        HWND hwnd{}; HDC hdc{}; HDC memdc{}; HBITMAP bmp{}; void* pixels{}; unsigned int pitch{}; unsigned int w{}; unsigned int h{};
    };

    Expect<void*> CreateCpuPresent(WindowHandle hwnd, UInt32 width, UInt32 height) noexcept {
        CpuPresentState* s = reinterpret_cast<CpuPresentState*>(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CpuPresentState)));
        if (!s) return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        s->hwnd = reinterpret_cast<HWND>(hwnd.Get());
        s->hdc = s->hwnd ? GetDC(s->hwnd) : nullptr;
        s->memdc = CreateCompatibleDC(s->hdc);
        s->w = width; s->h = height; s->pitch = s->w * 4;
        BITMAPINFO bmi{}; bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); bmi.bmiHeader.biWidth = static_cast<long>(s->w); bmi.bmiHeader.biHeight = -static_cast<long>(s->h);
        bmi.bmiHeader.biPlanes = 1; bmi.bmiHeader.biBitCount = 32; bmi.bmiHeader.biCompression = BI_RGB;
        void* pixels = nullptr;
        s->bmp = CreateDIBSection(s->memdc, &bmi, DIB_RGB_COLORS, &pixels, nullptr, 0);
        if (!s->bmp || !pixels) {
            if (s->bmp) DeleteObject(s->bmp);
            if (s->memdc) DeleteDC(s->memdc);
            if (s->hdc && s->hwnd) ReleaseDC(s->hwnd, s->hdc);
            ::HeapFree(::GetProcessHeap(), 0, s);
            return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        s->pixels = pixels; SelectObject(s->memdc, s->bmp);
        return Expect<void*>::Ok(reinterpret_cast<void*>(s));
    }
    Status DestroyCpuPresent(void* state) noexcept {
        CpuPresentState* s = reinterpret_cast<CpuPresentState*>(state);
        if (!s) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        if (s->bmp) DeleteObject(s->bmp);
        if (s->memdc) DeleteDC(s->memdc);
        if (s->hdc && s->hwnd) ReleaseDC(s->hwnd, s->hdc);
        ::HeapFree(::GetProcessHeap(), 0, s);
        return Ok(StatusDomain::System());
    }
    Expect<void*> CpuGetBuffer(void* state) noexcept {
        CpuPresentState* s = reinterpret_cast<CpuPresentState*>(state);
        if (!s || !s->pixels) return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        return Expect<void*>::Ok(s->pixels);
    }
    UInt32 CpuGetPitch(void* state) noexcept {
        CpuPresentState* s = reinterpret_cast<CpuPresentState*>(state); return s ? s->pitch : 0u;
    }
    Status CpuPresent(void* state) noexcept {
        CpuPresentState* s = reinterpret_cast<CpuPresentState*>(state);
        if (!s) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        if (s->hdc && s->memdc) { BitBlt(s->hdc, 0, 0, (int)s->w, (int)s->h, s->memdc, 0, 0, SRCCOPY); }
        return Ok(StatusDomain::System());
    }
}
