export module Prm.Element:Config;

// 统一的 Core 配置分区（单一模块声明与单一全局模块片段）
export namespace Prm {
    // 本模块内定义编译期回退值：若未通过 -D 定义，则采用安全缺省。
    #if defined(ENABLE_PRINT)
      #define ENABLE_PRINT_VALUE ((ENABLE_PRINT) != 0)
    #else
      #define ENABLE_PRINT_VALUE (1)
    #endif

    #if defined(ENABLE_STACKTRACE)
      #define ENABLE_STACKTRACE_VALUE ((ENABLE_STACKTRACE) != 0)
    #else
      #define ENABLE_STACKTRACE_VALUE (1)
    #endif

    #if defined(STRICT_ALLOC_GUARDS)
      #define STRICT_ALLOC_GUARDS_VALUE ((STRICT_ALLOC_GUARDS) != 0)
    #else
      #define STRICT_ALLOC_GUARDS_VALUE (0)
    #endif

    // 编译期开关导出
    inline constexpr bool kEnablePrint = (ENABLE_PRINT_VALUE);
    inline constexpr bool kEnableStacktrace = (ENABLE_STACKTRACE_VALUE);
    inline constexpr bool kStrictAllocatorGuards = (STRICT_ALLOC_GUARDS_VALUE);

    #if defined(_MSC_VER)
      inline constexpr bool kCompilerMsvc = true;
    #else
      inline constexpr bool kCompilerMsvc = false;
    #endif
    #if defined(__clang__)
      inline constexpr bool kCompilerClang = true;
    #else
      inline constexpr bool kCompilerClang = false;
    #endif
    #if defined(__GNUC__)
      inline constexpr bool kCompilerGcc = true;
    #else
      inline constexpr bool kCompilerGcc = false;
    #endif

    #if defined(_DEBUG) || defined(DEBUG)
      inline constexpr bool kIsDebug = true;
    #else
      inline constexpr bool kIsDebug = false;
    #endif

    // 额外的编译期配置查询（与全局构建选项一致）
    [[nodiscard]] consteval bool ExceptionsEnabled() noexcept {
    #if defined(CFG_EXCEPTIONS) && CFG_EXCEPTIONS
        return true;
    #else
        return false;
    #endif
    }

    [[nodiscard]] consteval bool RttiEnabled() noexcept {
    #if defined(CFG_RTTI) && CFG_RTTI
        return true;
    #else
        return false;
    #endif
    }

    // 清理本单元内部使用的中间宏，避免干扰其他单元
    #undef ENABLE_PRINT_VALUE
    #undef ENABLE_STACKTRACE_VALUE
    #undef STRICT_ALLOC_GUARDS_VALUE
}

#if defined(_MSC_VER)
  #ifdef _CPPUNWIND
    #error "Exceptions must be disabled (MSVC /EHsc- or /EHs- /EHa-)"
  #endif
  #ifdef _CPPRTTI
    #error "RTTI must be disabled (MSVC /GR-)"
  #endif
#elif defined(__clang__)
  #if defined(__EXCEPTIONS)
    #error "Exceptions must be disabled (clang -fno-exceptions)"
  #endif
  #if __has_feature(cxx_rtti)
    #error "RTTI must be disabled (clang -fno-rtti)"
  #endif
#elif defined(__GNUC__)
  #if defined(__EXCEPTIONS)
    #error "Exceptions must be disabled (gcc -fno-exceptions)"
  #endif
  #ifdef __GXX_RTTI
    #error "RTTI must be disabled (gcc -fno-rtti)"
  #endif
#endif
