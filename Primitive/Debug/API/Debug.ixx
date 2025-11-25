module;
export module Prm.Debug;

export namespace Prm {
    export class Debug {
    public:
        static void Break() noexcept;
        static Status Output(StringView text) noexcept;
    };
}
