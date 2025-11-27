export module Prm.Window:Ops;
import Element;
import Flow;
import Text;
import Paradigm;
import :Types;

export namespace Prm {
    export class Window {
    public:
        static Expect<WindowHandle> Create(const WindowDesc& desc, WndProcCallback callback) noexcept;
        static void Destroy(WindowHandle h) noexcept;
        static Status Show(WindowHandle h) noexcept;
        static Status Hide(WindowHandle h) noexcept;
        static Status SetTitle(WindowHandle h, StringView title) noexcept;
        static Status Resize(WindowHandle h, UInt32 w, UInt32 hgt) noexcept;
        static Status SetCursorMode(WindowHandle h, CursorMode m) noexcept;
        static Expect<void*> Native(WindowHandle h) noexcept;

        static bool ProcessOneMessage(WindowHandle h = WindowHandle{nullptr}) noexcept;
    };
}
