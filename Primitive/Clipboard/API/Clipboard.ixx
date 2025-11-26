module;
export module Prm.Clipboard;
import Element;
import Flow;

export namespace Prm {
    export class Clipboard {
    public:
        static Status SetText(Span<const Char8, DynamicExtent> text) noexcept;
        static Expect<USize> GetText(Span<Char8, DynamicExtent> buffer) noexcept;
    };
}
