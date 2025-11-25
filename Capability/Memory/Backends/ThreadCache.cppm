module Memory;
import Language;
import Platform;
import :ThreadCache;

namespace Memory {
    static thread_local ThreadLocalContext* t_tlsContext = nullptr;
    struct ThreadContextGuard {
        ThreadContextGuard() noexcept { ThreadMemory::Init(); }
        ~ThreadContextGuard() noexcept { ThreadMemory::Shutdown(); }
    };
    static thread_local ThreadContextGuard g_guard{};

    void ThreadMemory::Init() noexcept {
        if (t_tlsContext) return;
        auto h = Platform::Memory::Heap::GetProcessDefault();
        auto r = Platform::Memory::Heap::AllocRaw(h, sizeof(ThreadLocalContext));
        if (!r.IsOk()) return;
        void* raw = r.Value();
        t_tlsContext = new (raw) ThreadLocalContext{};
    }

    void ThreadMemory::Shutdown() noexcept {
        if (!t_tlsContext) return;
        auto h = Platform::Memory::Heap::GetProcessDefault();
        ThreadLocalContext* ctx = t_tlsContext;
        t_tlsContext = nullptr;
        ctx->~ThreadLocalContext();
        (void)Platform::Memory::Heap::FreeRaw(h, ctx);
    }

    ThreadLocalContext* ThreadMemory::Get() noexcept { if (!t_tlsContext) ThreadMemory::Init(); return t_tlsContext; }
}