export module Prm.Meta:TypeInfo;
import Prm.Element;

export namespace Prm {
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

    template<typename T>
    struct TypeInfo {
        static consteval const char* GetTypeName() noexcept { return detail::GetTypeSignature<T>(); }
    };
}

