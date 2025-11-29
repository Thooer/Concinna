module;
export module Lang.Text:Format;
import Lang.Element;
import Lang.Semantics;
import :StringView;
import :FormatArg;
import :FormatResult;
import <charconv>;
import <cstdint>;

export template<typename T> struct IsStringViewLike : FalseType {};
export template<> struct IsStringViewLike<StringView> : TrueType {};

export inline constexpr FormatArg MakeArg(Int8 v) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::I64; a.u.i64 = static_cast<Int64>(v); return a; }
export inline constexpr FormatArg MakeArg(Int16 v) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::I64; a.u.i64 = static_cast<Int64>(v); return a; }
export inline constexpr FormatArg MakeArg(Int32 v) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::I64; a.u.i64 = static_cast<Int64>(v); return a; }
export inline constexpr FormatArg MakeArg(Int64 v) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::I64; a.u.i64 = v; return a; }
export inline constexpr FormatArg MakeArg(UInt8 v) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::U64; a.u.u64 = static_cast<UInt64>(v); return a; }
export inline constexpr FormatArg MakeArg(UInt16 v) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::U64; a.u.u64 = static_cast<UInt64>(v); return a; }
export inline constexpr FormatArg MakeArg(UInt32 v) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::U64; a.u.u64 = static_cast<UInt64>(v); return a; }
export inline constexpr FormatArg MakeArg(UInt64 v) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::U64; a.u.u64 = v; return a; }
export inline constexpr FormatArg MakeArg(Float32 v) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::F64; a.u.f64 = static_cast<Float64>(v); return a; }
export inline constexpr FormatArg MakeArg(Float64 v) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::F64; a.u.f64 = v; return a; }
export inline constexpr FormatArg MakeArg(const char* s) noexcept { StringView sv{s}; FormatArg a{}; a.kind = FormatArg::Kind::Str; a.u.str.data = sv.data(); a.u.str.size = sv.size(); return a; }
export inline constexpr FormatArg MakeArg(StringView sv) noexcept { FormatArg a{}; a.kind = FormatArg::Kind::Str; a.u.str.data = sv.data(); a.u.str.size = sv.size(); return a; }


export template<typename T>
inline constexpr FormatArg MakeArg(T*) noexcept { StringView sv{""}; FormatArg a{}; a.kind = FormatArg::Kind::Str; a.u.str.data = sv.data(); a.u.str.size = sv.size(); return a; }


export template<USize N>
struct FormatPack {
    Array<FormatArg, N> args;
    [[nodiscard]] constexpr USize Size() const noexcept { return N; }
    [[nodiscard]] constexpr Span<const FormatArg, N> AsSpan() const noexcept { return Span<const FormatArg, N>(args.data(), N); }
};


export template<typename... Ts>
[[nodiscard]] constexpr auto CaptureArgs(Ts&&... ts) noexcept {
    constexpr USize N = static_cast<USize>(sizeof...(Ts));
    FormatPack<N> pack{};
    USize i = 0;
    ((pack.args[i++] = MakeArg(Forward<Ts>(ts))), ...);
    return pack;
}


export [[nodiscard]] inline FormatResult FormatTo(Span<Char8> out, StringView fmt, Span<const FormatArg> args) noexcept {
    FormatResult r{ FormatStatus::Ok, 0 };
    USize oi = 0;
    const Char8* f = fmt.data();
    USize fi = 0;
    USize ai = 0;
    const USize fn = fmt.size();
    auto put_char = [&](Char8 c) noexcept {
        if (oi < out.size()) { out[oi++] = c; } else { r.status = FormatStatus::Truncated; }
    };
    auto put_str = [&](const Char8* s, USize n) noexcept {
        for (USize k=0;k<n;++k) { if (oi < out.size()) { out[oi++] = s[k]; } else { r.status = FormatStatus::Truncated; break; } }
    };
    auto format_arg = [&](const FormatArg& a) noexcept {
        if (r.status == FormatStatus::Truncated) return;
        if (a.kind == FormatArg::Kind::Str) { put_str(a.u.str.data, a.u.str.size); return; }
        char buf[64];
        char* begin = buf;
        char* end = buf + 64;
        std::to_chars_result res{};
        if (a.kind == FormatArg::Kind::I64) { res = std::to_chars(begin, end, static_cast<long long>(a.u.i64)); }
        else if (a.kind == FormatArg::Kind::U64) { res = std::to_chars(begin, end, static_cast<unsigned long long>(a.u.u64)); }
        else { res = std::to_chars(begin, end, a.u.f64); }
        if (res.ec != std::errc{}) { r.status = FormatStatus::InvalidSpec; return; }
        USize n = static_cast<USize>(res.ptr - begin);
        for (USize k=0;k<n;++k) { if (oi < out.size()) { out[oi++] = static_cast<Char8>(begin[k]); } else { r.status = FormatStatus::Truncated; break; } }
    };
    while (fi < fn) {
        Char8 c = f[fi++];
        if (c == static_cast<Char8>('{')) {
            if (fi < fn && f[fi] == static_cast<Char8>('{')) { ++fi; put_char(static_cast<Char8>('{')); continue; }
            USize idx = ai;
            bool has_idx = false;
            if (fi < fn && f[fi] != static_cast<Char8>('}')) {
                USize val = 0;
                while (fi < fn && f[fi] != static_cast<Char8>('}')) {
                    Char8 d = f[fi++];
                    if (d < static_cast<Char8>('0') || d > static_cast<Char8>('9')) { r.status = FormatStatus::InvalidSpec; break; }
                    val = val * 10 + static_cast<USize>(static_cast<unsigned>(d - static_cast<Char8>('0')));
                    has_idx = true;
                }
                idx = val;
            }
            if (fi >= fn || f[fi] != static_cast<Char8>('}')) { r.status = FormatStatus::InvalidSpec; break; }
            ++fi;
            if (!has_idx) { ai += 1; }
            if (idx >= args.size()) { r.status = FormatStatus::InvalidSpec; break; }
            format_arg(args[idx]);
        } else if (c == static_cast<Char8>('}')) {
            if (fi < fn && f[fi] == static_cast<Char8>('}')) { ++fi; put_char(static_cast<Char8>('}')); }
            else { r.status = FormatStatus::InvalidSpec; break; }
        } else {
            put_char(c);
        }
        if (r.status == FormatStatus::Truncated) break;
    }
    r.written = oi;
    return r;
}
