/**
 * @file Utility.ixx
 * @brief Prm:Utility 纯语言语义工具集（无容器）。
 * @details 提供 Forward/Move/Exchange/BitCast/枚举位操作与 InPlace 标签等。
 */
module;
export module Prm.Semantics:Utility;
import Prm.Element;
import :Debug;
import <immintrin.h>;
import <type_traits>;
import <utility>;
import <cstddef>;
import <cstdint>;
import <cstring>;
import <bit>;
import <new>;
 

export namespace Prm {
    template<Enum U>
    constexpr std::underlying_type_t<U> ToUnderlying(U value) noexcept {
        return static_cast<std::underlying_type_t<U>>(value);
    }

    template<typename T>
    using UnderlyingType = std::underlying_type_t<T>;

    template<typename T, typename U = T>
    [[nodiscard]] constexpr T Exchange(T& object, U&& value) {
        T old = Move(object);
        object = Forward<U>(value);
        return old;
    }

    template<typename T>
    [[nodiscard]] constexpr T&& Forward(std::remove_reference_t<T>& t) noexcept { return static_cast<T&&>(t); }
    template<typename T>
    [[nodiscard]] constexpr T&& Forward(std::remove_reference_t<T>&& t) noexcept { static_assert(!std::is_lvalue_reference_v<T>); return static_cast<T&&>(t); }
    template<typename T>
    [[nodiscard]] constexpr std::remove_reference_t<T>&& Move(T&& t) noexcept { return static_cast<std::remove_reference_t<T>&&>(t); }
    template<typename To, typename From>
    requires (TriviallyCopyable<To> && TriviallyCopyable<From>)
    [[nodiscard]] constexpr To BitCast(const From& src) noexcept { static_assert(sizeof(To) == sizeof(From)); return std::bit_cast<To>(src); }

    struct InPlaceTag {};
    inline constexpr InPlaceTag InPlace{};

    template<class T>
    struct InPlaceType { using type = T; };
    template<class T>
    inline constexpr InPlaceType<T> InPlaceT{};

    // 最小策略组合桩：为 Reflection 与 Algorithms 提供默认钩子
    template<class... Policies>
    struct Compose {
        struct Instrument {
            static void OnBefore(const char*) noexcept {}
            static void OnAfter(const char*) noexcept {}
        };
        struct Reflection {
            static void OnTypeMismatch(const char*, const char*) noexcept {}
            static void OnMemberNotFound(const char*, const char*) noexcept {}
        };
    };



    struct Config {
    #if defined(DEBUG) && (DEBUG)
        static constexpr bool IsDebug = true;
    #else
        static constexpr bool IsDebug = false;
    #endif
    };

    inline void DebugBreak() noexcept { DEBUGBREAK(); }

    template<typename T>
    [[nodiscard]] inline T* Launder(T* p) noexcept {
        return std::launder(p);
    }

    export inline void Prefetch(const void* ptr) noexcept {
#if defined(_MSC_VER)
        _mm_prefetch(reinterpret_cast<const char*>(ptr), _MM_HINT_T0);
#else
        __builtin_prefetch(ptr, 0, 3);
#endif
    }

    export inline void PrefetchRW(const void* ptr, int rw, int locality) noexcept {
#if defined(_MSC_VER)
        _mm_prefetch(reinterpret_cast<const char*>(ptr), _MM_HINT_T0);
#else
        __builtin_prefetch(ptr, rw, locality);
#endif
    }
}
