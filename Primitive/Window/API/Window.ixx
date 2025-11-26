// Platform.Window — 窗口与消息泵原语（纯原语，无解释）
module;
export module Prm.Window;
import Element;
import Flow;
import Text;
import Paradigm;

export namespace Prm {
    

    struct WindowHandleTag;
    export using WindowHandle = StrongAlias<void*, WindowHandleTag>;

    export struct WindowDesc {
        UInt32 width{1280};
        UInt32 height{720};
        bool   resizable{true};
        bool   visible{true};
    };

    export enum class CursorMode : UInt32 { Normal = 0, Hidden, Locked };
    // 使用默认调用约定以保持跨编译器兼容；后端可自行桥接到本地 ABI。
    export using WndProcCallback = Int64(*)(void*, UInt32, UIntPtr, IntPtr);

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

        // 消息泵原语：处理一个队列中的消息，返回是否处理了任何消息
        static bool ProcessOneMessage(WindowHandle h = WindowHandle{nullptr}) noexcept;
    };
}
