export module Reflection:TypeInfo;
import Element;

namespace detail {
    template<typename T>
    consteval const char* GetTypeSignature() noexcept {
#if defined(_MSC_VER)
        return __FUNCSIG__;
#elif defined(__clang__) || defined(__GNUC__)
        return __PRETTY_FUNCTION__;
#else
        return "unknown";
#endif
    }
}

export template<typename T>
struct TypeInfo {
    static consteval const char* GetTypeName() noexcept { return detail::GetTypeSignature<T>(); }
};
