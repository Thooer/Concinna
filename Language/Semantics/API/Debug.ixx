module;
export module Semantics:Debug;
import <cstddef>;

export using AssertHandler = void(*)(const char* condition, const char* message, const char* file, int line, const char* function);
    namespace detail {
        inline AssertHandler& AssertHandlerStorage() noexcept {
            static AssertHandler handler = nullptr;
            return handler;
        }
    }
    export inline void SetAssertHandler(AssertHandler handler) noexcept { detail::AssertHandlerStorage() = handler; }
    export [[nodiscard]] inline AssertHandler GetAssertHandler() noexcept { return detail::AssertHandlerStorage(); }

    export inline void DEBUGBREAK() noexcept {
#if defined(_MSC_VER)
        __debugbreak();
#else
        __builtin_trap();
#endif
    }

    export inline void CPU_RELAX() noexcept {
#if defined(__aarch64__) || defined(__arm__)
        asm volatile("yield" ::: "memory");
#else
        (void)0;
#endif
    }

    export inline void ASSERT(bool condition, const char* message = "") noexcept {
#if defined(_DEBUG) || defined(DEBUG)
        if (!condition) {
            if (auto handler = GetAssertHandler()) {
                handler("assert", message, __FILE__, __LINE__, __func__);
            }
            DEBUGBREAK();
        }
#else
        (void)condition; (void)message;
#endif
    }

    export inline void CHECK(bool condition, const char* message = "") noexcept {
#if defined(_DEBUG) || defined(DEBUG)
        ASSERT(condition, message);
#else
        (void)sizeof(condition); (void)message;
#endif
    }
    export inline void ENSURE(bool condition, const char* message = "") noexcept {
#if defined(_DEBUG) || defined(DEBUG)
        ASSERT(condition, message);
#else
        (void)sizeof(condition); (void)message;
#endif
    }

    export inline void UNREACHABLE() noexcept {
#if defined(_MSC_VER)
        __assume(0);
#else
        __builtin_unreachable();
#endif
    }
