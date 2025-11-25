module;
export module Prm.File;

export namespace Prm {
    export class FilePath {
    public:
        static Expect<USize> Normalize(StringView path, Span<Char8, DynamicExtent> out) noexcept;
        static Expect<USize> Join(StringView a, StringView b, Span<Char8, DynamicExtent> out) noexcept;
        static Expect<USize> Basename(StringView path, Span<Char8, DynamicExtent> out) noexcept;
    };
}
