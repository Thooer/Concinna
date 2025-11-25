module;
export module Prm.Debug;
import Prm.Element;
import Prm.Flow;
import Prm.Text;

export namespace Prm {
    export class Debug {
    public:
        static void Break() noexcept;
        static Status Output(StringView text) noexcept;
    };
}
