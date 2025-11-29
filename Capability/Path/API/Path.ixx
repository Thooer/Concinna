module;
export module Cap.Path;
import Lang;

export namespace Cap {
    export class Path {
    public:
        static Expect<USize> Normalize(Span<const Char8, DynamicExtent> inPath, Span<Char8, DynamicExtent> out) noexcept;
        static Expect<USize> Join(Span<const Char8, DynamicExtent> a, Span<const Char8, DynamicExtent> b, Span<Char8, DynamicExtent> out) noexcept;
        static Expect<USize> Basename(Span<const Char8, DynamicExtent> inPath, Span<Char8, DynamicExtent> out) noexcept;
        static Expect<USize> Extension(Span<const Char8, DynamicExtent> inPath, Span<Char8, DynamicExtent> out) noexcept;
    };
}

