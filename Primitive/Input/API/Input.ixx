module;
export module Prm.Input;
import Element;
import Flow;

export namespace Prm {
    export class Input {
    public:
        static Status Initialize() noexcept;
        static void Shutdown() noexcept;
    };

    export struct MouseState { Int32 x{0}; Int32 y{0}; bool left{false}; bool right{false}; bool middle{false}; };
    export struct KeyboardState { bool keys[256]{}; };

    export class Mouse {
    public:
        static Expect<MouseState> GetState() noexcept;
    };

    export class Keyboard {
    public:
        static Expect<KeyboardState> GetState() noexcept;
    };
}
