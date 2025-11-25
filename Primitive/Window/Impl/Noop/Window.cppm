module;
module Prm.Window;

import :Window;

namespace Prm {
    

    Expect<WindowHandle> Window::Create(const WindowDesc&, WndProcCallback) noexcept {
        return Expect<WindowHandle>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    void Window::Destroy(WindowHandle) noexcept {}
    Status Window::Show(WindowHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status Window::Hide(WindowHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status Window::SetTitle(WindowHandle, StringView) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status Window::Resize(WindowHandle, UInt32, UInt32) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status Window::SetCursorMode(WindowHandle, CursorMode) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Expect<void*> Window::Native(WindowHandle) noexcept { return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
    bool Window::ProcessOneMessage(WindowHandle) noexcept { return false; }
}
