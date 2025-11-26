
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <new> // 必须包含，用于 Expect 模板中的 placement new
import Prm.Window;
import Element;
import Flow;
import Paradigm;
import Text;

using namespace Prm;

namespace Prm {

    // 适配 Windows API 的回调签名
    using WndProcFn = Int64(__stdcall*)(void*, unsigned int, UInt64, Int64);

    // 全局存储用户回调
    static WndProcCallback g_userWndProc{nullptr};

    // 静态桥接函数
    static Int64 __stdcall _WndProcBridge(void* hWnd, unsigned int Msg, UInt64 wParam, Int64 lParam) {
        auto cb = g_userWndProc;
        if (cb) {
            return cb(hWnd, static_cast<UInt32>(Msg), static_cast<UIntPtr>(wParam), static_cast<IntPtr>(lParam));
        }
        return DefWindowProcA(reinterpret_cast<HWND>(hWnd), Msg, static_cast<WPARAM>(wParam), static_cast<LPARAM>(lParam));
    }

   Expect<WindowHandle> Window::Create(const WindowDesc& desc, WndProcCallback callback) noexcept {
        if (!callback) {
            return Expect<WindowHandle>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }

        WNDCLASSA wc{};
        wc.style        = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        g_userWndProc   = callback;
        wc.lpfnWndProc  = reinterpret_cast<WNDPROC>(&_WndProcBridge);
        wc.cbClsExtra   = 0;
        wc.cbWndExtra   = 0;
        wc.hInstance    = GetModuleHandleA(nullptr);
        wc.hIcon        = nullptr;
        wc.hCursor      = nullptr;
        wc.hbrBackground= nullptr;
        wc.lpszMenuName = nullptr;
        wc.lpszClassName= "EngineWindow";

        const ATOM atom = RegisterClassA(&wc);
        (void)atom;

        const unsigned long style = WS_OVERLAPPEDWINDOW | (desc.visible ? WS_VISIBLE : 0);
        HWND hWnd = CreateWindowExA(0, wc.lpszClassName, "Nova Engine",
                                     style, 100, 100,
                                     static_cast<int>(desc.width), static_cast<int>(desc.height),
                                     nullptr, nullptr, wc.hInstance, nullptr);
        if (!hWnd) {
            return Expect<WindowHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        if (desc.visible) {
            ShowWindow(hWnd, SW_SHOW);
            UpdateWindow(hWnd);
        }
        return Expect<WindowHandle>::Ok(WindowHandle{reinterpret_cast<void*>(hWnd)});
    }

   void Window::Destroy(WindowHandle h) noexcept {
        if (h.Get()) (void)DestroyWindow(reinterpret_cast<HWND>(h.Get()));
    }

   Status Window::Show(WindowHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = ShowWindow(reinterpret_cast<HWND>(h.Get()), SW_SHOW);
        (void)UpdateWindow(reinterpret_cast<HWND>(h.Get()));
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

   Status Window::Hide(WindowHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = ShowWindow(reinterpret_cast<HWND>(h.Get()), SW_HIDE);
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Status Window::SetTitle(WindowHandle h, StringView title) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        // 使用简单的栈缓冲转换，避免 unsafe 转换
        if (title.size() < 512) {
            char buf[512];
            for(USize i=0; i<title.size(); ++i) buf[i] = static_cast<char>(title[i]);
            buf[title.size()] = '\0';
            const int ok = SetWindowTextA(reinterpret_cast<HWND>(h.Get()), buf);
            return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
        }
        return Err(StatusDomain::System(), StatusCode::OutOfRange);
    }

    Status Window::Resize(WindowHandle h, UInt32 w, UInt32 hgt) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = MoveWindow(reinterpret_cast<HWND>(h.Get()), 0, 0, static_cast<int>(w), static_cast<int>(hgt), 1);
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    bool Window::ProcessOneMessage(WindowHandle h) noexcept {
        MSG msg{};
        HWND hwnd = h.Get() ? reinterpret_cast<HWND>(h.Get()) : nullptr;
        if (PeekMessageA(&msg, hwnd, 0u, 0u, PM_REMOVE)) {
            (void)TranslateMessage(&msg);
            (void)DispatchMessageA(&msg);
            return true;
        }
        return false;
    }

    Status Window::SetCursorMode(WindowHandle /*h*/, CursorMode m) noexcept {
        if (m == CursorMode::Hidden) {
            ShowCursor(0);
        } else {
            ShowCursor(1);
        }
        return Ok(StatusDomain::System());
    }

    Expect<void*> Window::Native(WindowHandle h) noexcept {
        return h.Get() ? Expect<void*>::Ok(h.Get()) : Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
    }
}

// ---- C ABI 稳定入口 ----

extern "C" __declspec(dllexport) void* Prm_Window_Create(const Prm::WindowDesc* desc, Prm::WndProcCallback cb) noexcept {
    const Prm::WindowDesc d = desc ? *desc : Prm::WindowDesc{};
    auto r = Prm::Window::Create(d, cb);
    return r.IsOk() ? r.Value().Get() : nullptr;
}

extern "C" __declspec(dllexport) void Prm_Window_Destroy(void* h) noexcept {
    Prm::Window::Destroy(Prm::WindowHandle{h});
}

extern "C" __declspec(dllexport) int Prm_Window_SetTitle(void* h, const char* title) noexcept {
    if (!h || !title) return 1;
    USize len = 0;
    while (title[len] != '\0') ++len;
    const auto sv = StringView{reinterpret_cast<const Char8*>(title), len};
    const auto s = Window::SetTitle(WindowHandle{h}, sv);
    return s.Ok() ? 0 : 1;
}

extern "C" __declspec(dllexport) int Prm_Window_ProcessOneMessage(void* h) noexcept {
    const bool processed = Prm::Window::ProcessOneMessage(Prm::WindowHandle{h});
    return processed ? 1 : 0;
}

extern "C" __declspec(dllexport) int Prm_Window_HasQuit() noexcept {
    MSG msg{};
    const unsigned int has = PeekMessageA(&msg, nullptr, WM_QUIT, WM_QUIT, 0u);
    return has ? 1 : 0;
}