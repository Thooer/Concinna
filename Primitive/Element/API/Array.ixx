export module Prm.Element:Array;
import :Types;

export namespace Prm {
    template<typename T, USize N>
    struct Array {
        T m_data[N];
        [[nodiscard]] static constexpr USize Size() noexcept { return N; }
        [[nodiscard]] constexpr T* data() noexcept { return m_data; }
        [[nodiscard]] constexpr const T* data() const noexcept { return m_data; }
        [[nodiscard]] constexpr T& operator[](USize i) noexcept { return m_data[i]; }
        [[nodiscard]] constexpr const T& operator[](USize i) const noexcept { return m_data[i]; }
        [[nodiscard]] constexpr T* begin() noexcept { return m_data; }
        [[nodiscard]] constexpr T* end() noexcept { return m_data + N; }
        [[nodiscard]] constexpr const T* begin() const noexcept { return m_data; }
        [[nodiscard]] constexpr const T* end() const noexcept { return m_data + N; }
    };
}

