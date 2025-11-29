module;
export module Lang.Text:StringView;
import Lang.Element;
import Lang.Semantics;


export struct StringView {
    using size_type = USize;
    using value_type = Char8;
    using pointer = const Char8*;

    constexpr StringView() noexcept : m_data(nullptr), m_size(0) {}
    constexpr StringView(pointer data, size_type count) noexcept : m_data(data), m_size(count) {}

    constexpr explicit StringView(const char* cstr) noexcept : m_data(BitCast<pointer>(cstr)), m_size(0) {
        if (cstr) {
            const Char8* p = m_data;
            while (*p != static_cast<Char8>(0)) { ++p; }
            m_size = static_cast<size_type>(p - m_data);
        }
    }
    constexpr StringView(const char* data, size_type count) noexcept : m_data(BitCast<pointer>(data)), m_size(count) {}

    // 从以 '\0' 结尾的 UTF-8 字符串构造（运行时长度扫描）
    constexpr explicit StringView(pointer cstr) noexcept : m_data(cstr), m_size(0) {
        if (cstr) {
            const Char8* p = cstr;
            while (*p != static_cast<Char8>(0)) { ++p; }
            m_size = static_cast<size_type>(p - cstr);
        }
    }

    [[nodiscard]] constexpr size_type size() const noexcept { return m_size; }
    [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }
    [[nodiscard]] constexpr pointer data() const noexcept { return m_data; }

    [[nodiscard]] constexpr const Char8& operator[](USize idx) const noexcept { return m_data[idx]; }
    [[nodiscard]] constexpr const Char8* begin() const noexcept { return m_data; }
    [[nodiscard]] constexpr const Char8* end() const noexcept { return m_data + m_size; }

    [[nodiscard]] constexpr bool Equals(StringView other) const noexcept {
        if (m_size != other.m_size) return false;
        for (USize i=0;i<m_size;++i) { if (m_data[i]!=other.m_data[i]) return false; }
        return true;
    }
    [[nodiscard]] constexpr bool StartsWith(StringView prefix) const noexcept {
        if (prefix.m_size > m_size) return false;
        for (USize i=0;i<prefix.m_size;++i) { if (m_data[i]!=prefix.m_data[i]) return false; }
        return true;
    }
    [[nodiscard]] constexpr bool EndsWith(StringView suffix) const noexcept {
        if (suffix.m_size > m_size) return false;
        const USize off = m_size - suffix.m_size;
        for (USize i=0;i<suffix.m_size;++i) { if (m_data[off+i]!=suffix.m_data[i]) return false; }
        return true;
    }
    [[nodiscard]] constexpr SSize Find(StringView needle) const noexcept {
        if (needle.m_size == 0) return 0;
        if (needle.m_size > m_size) return -1;
        for (USize i=0; i + needle.m_size <= m_size; ++i) {
            bool match=true;
            for (USize j=0;j<needle.m_size;++j) { if (m_data[i+j]!=needle.m_data[j]) { match=false; break; } }
            if (match) return static_cast<SSize>(i);
        }
        return -1;
    }

    struct SplitRange {
        struct EndSentinel {};
        struct Iterator {
            const Char8* cur;
            const Char8* end;
            Char8 delim;
            const Char8* token_begin;
            size_type token_len;
            bool done;
            Iterator(const Char8* s, const Char8* e, Char8 d) noexcept
                : cur(s), end(e), delim(d), token_begin(nullptr), token_len(0), done(false) { advance(); }
            void advance() noexcept {
                if (cur == nullptr || cur >= end) { done = true; token_begin = nullptr; token_len = 0; return; }
                const Char8* start = cur;
                while (cur < end && *cur != delim) { ++cur; }
                token_begin = start;
                token_len = static_cast<size_type>(cur - start);
                if (cur < end) { ++cur; } else { /* at end */ }
            }
            [[nodiscard]] StringView operator*() const noexcept { return StringView(token_begin, token_len); }
            Iterator& operator++() noexcept { advance(); return *this; }
            friend bool operator!=(const Iterator& it, EndSentinel) noexcept { return !it.done; }
        };
        const Char8* s_begin;
        size_type s_len;
        Char8 delim;
        SplitRange(StringView sv, Char8 d) noexcept : s_begin(sv.data()), s_len(sv.size()), delim(d) {}
        [[nodiscard]] Iterator begin() const noexcept { return Iterator(s_begin, s_begin + s_len, delim); }
        [[nodiscard]] EndSentinel end() const noexcept { return EndSentinel{}; }
    };

    [[nodiscard]] SplitRange Split(Char8 delim) const noexcept { return SplitRange(*this, delim); }

private:
    pointer   m_data;
    size_type m_size;
};

