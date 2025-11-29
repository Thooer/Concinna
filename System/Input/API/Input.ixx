export module Sys.Input;
import Lang.Element;
import Lang.Flow;
import Prm.Window;

void RawEventBridge(void* hWnd, UInt32 msg, UIntPtr wParam, IntPtr lParam) noexcept;
namespace Sys { Status OnWindowEvent(void* hWnd, UInt32 msg, UIntPtr wParam, IntPtr lParam) noexcept; }

export namespace Sys {
    Status RegisterWindow(void* nativeWindowHandle) noexcept {
        Prm::Window::SetRawEventCallback(&RawEventBridge);
        return Ok(StatusDomain::System());
    }
    Status UnregisterWindow() noexcept {
        return Ok(StatusDomain::System());
    }
    Status OnWindowEvent(void* /*hWnd*/, UInt32 msg, UIntPtr wParam, IntPtr lParam) noexcept {
        (void)msg; (void)wParam; (void)lParam;
        return Ok(StatusDomain::System());
    }
}

void RawEventBridge(void* hWnd, UInt32 msg, UIntPtr wParam, IntPtr lParam) noexcept {
    (void)Sys::OnWindowEvent(hWnd, msg, wParam, lParam);
}
