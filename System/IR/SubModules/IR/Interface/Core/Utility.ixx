export module Foundation.IRSystem:IR.Core.Utility;
import Foundation.IRSystem:IR.Core.Types;
import Foundation.IRSystem:IR.Core.TypeMeta;
import <type_traits>;
import <bit>;
import <utility>;
import <cstddef>;
import <cstdint>;
import <cstring>;
import <new>;

export namespace Foundation::IRSystem {
    template<typename T>
    [[nodiscard]] constexpr T&& Forward(std::remove_reference_t<T>& t) noexcept { return static_cast<T&&>(t); }
    template<typename T>
    [[nodiscard]] constexpr T&& Forward(std::remove_reference_t<T>&& t) noexcept { static_assert(!std::is_lvalue_reference_v<T>); return static_cast<T&&>(t); }
    template<typename T>
    [[nodiscard]] constexpr std::remove_reference_t<T>&& Move(T&& t) noexcept { return static_cast<std::remove_reference_t<T>&&>(t); }
    template<typename T>
    [[nodiscard]] constexpr T* AddrOf(T& t) noexcept { return std::addressof(t); }
    template<typename T>
    [[nodiscard]] constexpr T& Deref(T* p) noexcept { return *p; }
    template<typename T, typename... Args>
    [[nodiscard]] inline T* ConstructAt(T* p, Args&&... args) noexcept(noexcept(::new (static_cast<void*>(p)) T(Forward<Args>(args...)))) { return ::new (static_cast<void*>(p)) T(Forward<Args>(args...)); }
    template<typename T>
    inline void DestroyAt(T* p) noexcept { if (p) { p->~T(); } }
    inline void CopyBytes(void* dst, const void* src, std::size_t n) noexcept { (void)std::memcpy(dst, src, n); }
    inline void FillBytes(void* dst, std::uint8_t value, std::size_t n) noexcept { (void)std::memset(dst, static_cast<int>(value), n); }
    template<typename To, typename From> requires (std::is_trivially_copyable_v<To> && std::is_trivially_copyable_v<From> && sizeof(To) == sizeof(From))
    [[nodiscard]] constexpr To BitCast(const From& src) noexcept { return std::bit_cast<To>(src); }
    template<typename T>
    [[nodiscard]] constexpr const T& AsConst(T& t) noexcept { return t; }
    template<typename T>
    [[nodiscard]] constexpr bool IsNull(T* p) noexcept { return p == nullptr; }
    template<typename T>
    [[nodiscard]] constexpr T* NullPtr() noexcept { return nullptr; }
    inline constexpr void Assume(bool cond) noexcept { (void)cond; }
}