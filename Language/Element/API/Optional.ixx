export module Lang.Element:Optional;
import :Types;
import <utility>;


struct NullOptT { explicit constexpr NullOptT(int) noexcept {} };
export inline constexpr NullOptT NullOpt{0};

export template<typename T>
struct Optional {
    bool m_has{false};
    union U { T value; U(){} ~U(){} } u;
    constexpr Optional() noexcept : m_has(false), u{} {}
    Optional(const Optional&) = delete;
    Optional& operator=(const Optional&) = delete;
    constexpr Optional(Optional&& other) noexcept : m_has(other.m_has) {
        if (m_has) { new (&u.value) T(std::move(other.u.value)); other.u.value.~T(); other.m_has = false; }
    }
    constexpr Optional& operator=(Optional&& other) noexcept {
        if (this == &other) return *this;
        Reset();
        m_has = other.m_has;
        if (m_has) { new (&u.value) T(std::move(other.u.value)); other.u.value.~T(); other.m_has = false; }
        return *this;
    }
    ~Optional() noexcept { if (m_has) { u.value.~T(); } }
    [[nodiscard]] constexpr bool Has() const noexcept { return m_has; }
    template<typename... Args>
    constexpr void Emplace(Args&&... args) noexcept(IsNothrowConstructible<T, Args...>) {
        Reset();
        new (&u.value) T(std::forward<Args>(args)...);
        m_has = true;
    }
    constexpr void Reset() noexcept { if (m_has) { u.value.~T(); m_has = false; } }
    [[nodiscard]] constexpr T& ValueRef() & noexcept {
        if (!m_has) {
#if defined(_MSC_VER)
            __debugbreak();
#else
            __builtin_trap();
#endif
        }
        return u.value;
    }
    [[nodiscard]] constexpr const T& ValueRef() const& noexcept {
        if (!m_has) {
#if defined(_MSC_VER)
            __debugbreak();
#else
            __builtin_trap();
#endif
        }
        return u.value;
    }
    [[nodiscard]] constexpr T Take() noexcept(NothrowMoveConstructible<T>) {
        if (!m_has) {
#if defined(_MSC_VER)
            __debugbreak();
#else
            __builtin_trap();
#endif
        }
        m_has=false; return std::move(u.value);
    }
};
