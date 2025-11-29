module;
export module Prm.HID;
import Lang.Element;
import Lang.Flow;

export namespace Prm {
    export struct KeyboardState {
        UInt8 keys[256]{};
    };

    export struct MouseState {
        Int32 dx{0};
        Int32 dy{0};
        Int32 wheel{0};
        UInt32 buttons{0};
    };

    export struct GamepadState {
        UInt32 buttons{0};
        float  axes[8]{};
    };

    export class HID {
    public:
        static Status RegisterDevices(void* nativeWindowHandle) noexcept;
        static Status UnregisterDevices() noexcept;
        static Status UpdateFromRawEvent(const void* data, USize size) noexcept;
        static Expect<KeyboardState> GetKeyboardState() noexcept;
        static Expect<MouseState>    GetMouseState() noexcept;
        static Expect<GamepadState>  GetGamepadState(UInt32 index) noexcept;
    };
}

