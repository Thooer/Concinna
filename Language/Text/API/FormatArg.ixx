export module Lang.Text:FormatArg;
import Lang.Element;

export struct FormatArg {
    enum class Kind : UInt8 { I64, U64, F64, Str } kind;
    union {
        Int64 i64;
        UInt64 u64;
        Float64 f64;
        struct { const Char8* data; USize size; } str;
    } u;
};

