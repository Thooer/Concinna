module Cap.Memory;
import Lang;
import Prm.Ownership;
import :ThreadCache;

namespace Cap {
    static thread_local ThreadLocalContext* t_tlsContext = nullptr;

    void ThreadMemory::Init() noexcept {
        if (t_tlsContext) return;
        void* raw = ::operator new(sizeof(ThreadLocalContext));
        t_tlsContext = new (raw) ThreadLocalContext{};
    }

    void ThreadMemory::Shutdown() noexcept {
        if (!t_tlsContext) return;
        ThreadLocalContext* ctx = t_tlsContext;
        t_tlsContext = nullptr;
        ctx->~ThreadLocalContext();
        ::operator delete(ctx);
    }

    ThreadLocalContext* ThreadMemory::Get() noexcept { if (!t_tlsContext) ThreadMemory::Init(); return t_tlsContext; }
}
