module;
export module Prm.File;
import Element;
import Flow;

export namespace Prm {
    export class FilePath {
    public:
        static Expect<USize> Normalize(Span<const Char8, DynamicExtent> path, Span<Char8, DynamicExtent> out) noexcept;
        static Expect<USize> Join(Span<const Char8, DynamicExtent> a, Span<const Char8, DynamicExtent> b, Span<Char8, DynamicExtent> out) noexcept;
        static Expect<USize> Basename(Span<const Char8, DynamicExtent> path, Span<Char8, DynamicExtent> out) noexcept;
    };
}
