module;
export module Prm.Debug;
import Lang.Element;
import Lang.Flow;

export namespace Prm {
    export class Debug {
    public:
        static void Break() noexcept;
        static Status Output(Span<const Char8, DynamicExtent> text) noexcept;
    };
}
