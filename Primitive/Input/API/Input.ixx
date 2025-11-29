module;
export module Prm.Input;
import Lang.Element;
import Lang.Flow;

export namespace Prm {
    export class Input {
    public:
        static Status Initialize() noexcept;
        static void Shutdown() noexcept;
    };

    export struct GamepadState {
        UInt32 buttons{0};
        float  axes[8]{};
    };
    export class Gamepad {
    public:
        static Expect<GamepadState> GetState(UInt32 index) noexcept;
    };
}
