module;
export module Platform:Clipboard;

import Prm;

export namespace Platform {
    export class Clipboard {
    public:
        static Status SetText(StringView text) noexcept;
        static Expect<USize> GetText(Span<Char8, DynamicExtent> buffer) noexcept;
    };
}

