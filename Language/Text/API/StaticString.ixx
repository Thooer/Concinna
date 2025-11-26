module;
export module Text:StaticString;
import Element;
import Semantics;
import :StringView;
import :Format;


export template<USize N>
struct StaticString {
    Char8 m_buf[N];
    USize m_size{0};
    [[nodiscard]] constexpr USize size() const noexcept { return m_size; }
    [[nodiscard]] static constexpr USize capacity() noexcept { return N; }
    [[nodiscard]] constexpr const Char8* data() const noexcept { return m_buf; }
    [[nodiscard]] constexpr Char8* data() noexcept { return m_buf; }
    constexpr void Clear() noexcept { m_size = 0; }
    constexpr void Append(Char8 c) noexcept { if (m_size < N) { m_buf[m_size++] = c; } }
    constexpr void Append(StringView sv) noexcept {
        const USize n = sv.size();
        const USize r = (N > m_size) ? (N - m_size) : 0;
        const USize w = (n < r) ? n : r;
        for (USize i=0;i<w;++i) { m_buf[m_size++] = sv[i]; }
    }
    constexpr void Assign(StringView sv) noexcept { m_size = 0; Append(sv); }
    [[nodiscard]] constexpr StringView View() const noexcept { return StringView(m_buf, m_size); }
    [[nodiscard]] constexpr Span<Char8> TailSpan() noexcept { return Span<Char8>(m_buf + m_size, (N > m_size) ? (N - m_size) : 0); }
    [[nodiscard]] constexpr Span<const Char8> TailSpan() const noexcept { return Span<const Char8>(m_buf + m_size, (N > m_size) ? (N - m_size) : 0); }
    inline FormatResult AppendFormat(StringView fmt, Span<const FormatArg> args) noexcept {
        auto out = TailSpan();
        auto res = FormatTo(out, fmt, args);
        const USize adv = (res.written <= out.size()) ? res.written : out.size();
        m_size += adv;
        return res;
    }
};
