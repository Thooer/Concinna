export module Prm.Window:Types;
import Lang.Element;
import Lang.Paradigm;

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
    export using WndProcCallback = Int64(*)(void*, UInt32, UIntPtr, IntPtr);
    export using RawEventCallback = void(*)(void*, UInt32, UIntPtr, IntPtr);
}
