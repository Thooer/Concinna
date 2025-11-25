module;
export module Prm.Clipboard;
import Prm.Element;
import Prm.Flow;
import Prm.Text;

export namespace Prm {
    export class Clipboard {
    public:
        static Status SetText(StringView text) noexcept;
        static Expect<USize> GetText(Span<Char8, DynamicExtent> buffer) noexcept;
    };
}
