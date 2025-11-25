export module Prm.Text:FormatResult;
import Prm.Element;

export namespace Prm {
    enum class FormatStatus : UInt8 { Ok, Truncated, InvalidSpec };
    struct FormatResult { FormatStatus status; USize written; };
}

