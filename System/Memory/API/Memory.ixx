export module System.Memory;

export namespace Sys {
    export void InitThreadMemory() noexcept;
    export void ShutdownThreadMemory() noexcept;
}

namespace Sys {
    void InitThreadMemory() noexcept {}
    void ShutdownThreadMemory() noexcept {}
}
