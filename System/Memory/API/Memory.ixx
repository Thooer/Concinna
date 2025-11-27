export module System.Memory;
import Cap.Memory;

export namespace Sys {
    export void InitThreadMemory() noexcept;
    export void ShutdownThreadMemory() noexcept;
}

namespace Sys {
    void InitThreadMemory() noexcept { Memory::ThreadMemory::Init(); }
    void ShutdownThreadMemory() noexcept { Memory::ThreadMemory::Shutdown(); }
}
