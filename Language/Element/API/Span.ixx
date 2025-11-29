export module Lang.Element:Span;
import :Types;


export template<typename T, USize Extent = DynamicExtent, typename... Ps>
struct Span {
    using element_type = T;
    using value_type = RemoveCvRef<T>;
    using size_type = USize;
    using pointer = T*;
    using reference = T&;

    static constexpr bool kStaticExtent = (Extent != DynamicExtent);
    static constexpr USize extent = Extent;

    constexpr Span() noexcept : m_data(nullptr), m_size(kStaticExtent ? Extent : 0) {}
    constexpr Span(pointer data, USize count) noexcept : m_data(data), m_size(kStaticExtent ? Extent : count) {}

    template<typename U>
        requires SameAs<RemoveCvRef<U>, RemoveCvRef<T>>
    constexpr Span(Span<U, Extent, Ps...> other) noexcept
        : m_data(const_cast<pointer>(other.data())), m_size(other.size()) {}

    [[nodiscard]] constexpr size_type size() const noexcept { return m_size; }
    [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }
    [[nodiscard]] constexpr pointer data() const noexcept { return m_data; }

    [[nodiscard]] constexpr reference operator[](USize idx) const noexcept { return m_data[idx]; }

    [[nodiscard]] constexpr pointer begin() const noexcept { return m_data; }
    [[nodiscard]] constexpr pointer end() const noexcept { return m_data + m_size; }

    [[nodiscard]] constexpr Span Slice(USize offset, USize count) const noexcept {
        if (offset > m_size) return Span{};
        const USize avail = m_size - offset;
        const USize n = (count > avail) ? avail : count;
        return Span{ m_data + offset, n };
    }
    [[nodiscard]] constexpr Span Subspan(USize offset) const noexcept {
        if (offset > m_size) return Span{};
        return Span{ m_data + offset, m_size - offset };
    }

private:
    pointer m_data;
    USize   m_size;
};
