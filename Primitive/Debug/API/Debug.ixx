module;
export module Platform:Debug;

import Prm;

export namespace Platform {
    export class Debug {
    public:
        static void Break() noexcept;
        static Status Output(StringView text) noexcept;
    };
}

