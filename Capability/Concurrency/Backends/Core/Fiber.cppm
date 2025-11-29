module Cap.Concurrency;
import Lang;
import Cap.Memory;
import Prm.Threading;
import Prm.Time;
import Prm.Ownership;
import :Fiber;
import Tools.Tracy;

extern "C" void Nova_FiberEnter() noexcept;
extern "C" void Nova_FiberExit() noexcept;

namespace Cap {
    FiberStackPool::FiberStackPool(USize capacity) noexcept {
        auto r = Prm::VirtualMemory::Reserve(capacity);
        if (r.IsOk()) { m_base = r.Value(); m_capacity = capacity; }
    }

    FiberStackPool::~FiberStackPool() noexcept {
        if (m_base) { (void)Prm::VirtualMemory::Release(m_base); m_base = nullptr; }
        m_capacity = 0; m_offset = 0; m_committed = 0;
    }

    [[nodiscard]] FiberStack FiberStackPool::Alloc() noexcept {
        FiberStack fs{};
        auto* n = m_free.Pop();
        if (n) {
            fs.base = n->ptr;
            fs.size = m_chunk;
            auto h = Prm::Heap::GetProcessDefault();
            (void)Prm::Heap::FreeRaw(h, n);
            return fs;
        }
        USize inc = m_chunk;
#if defined(_DEBUG)
        inc += Prm::VirtualMemory::PageSize();
#endif
        if (!m_base || (m_offset + inc > m_capacity)) { return fs; }
        auto c = Prm::VirtualMemory::Commit(static_cast<char*>(m_base) + m_offset, m_chunk);
        if (!c.Ok()) { return fs; }
        fs.base = static_cast<char*>(m_base) + m_offset;
        fs.size = m_chunk;
        m_offset += inc;
        m_committed += m_chunk;
        return fs;
    }

    void FiberStackPool::Free(FiberStack s) noexcept {
        if (!s.base) return;
        auto h = Prm::Heap::GetProcessDefault();
        auto rn = Prm::Heap::AllocRaw(h, sizeof(FreeNode));
        if (!rn.IsOk()) return;
        void* mem = rn.Value();
        auto* n = new (mem) FreeNode{};
        n->ptr = s.base;
        m_free.Push(n);
    }
    void Fiber::Setup(FiberStackPool& pool, FiberFunc fn, void* a, void* returnCtx) noexcept {
        stack = pool.Alloc();
        poolRef = &pool;
        entry = fn;
        arg = a;
        retCtx = returnCtx;
        state = FiberState::Ready;
        if (!stack.base) { state = FiberState::Dead; return; }
        char* base = static_cast<char*>(stack.base);
        USize ps = Prm::VirtualMemory::PageSize();
        char* safeTop = base + stack.size - ps;
        USize sp = reinterpret_cast<USize>(safeTop);
        sp &= ~static_cast<USize>(0xF);
        USize color = (reinterpret_cast<USize>(base) ^ reinterpret_cast<USize>(this)) & static_cast<USize>(0x3FF);
        color &= ~static_cast<USize>(0xF);
        sp -= (32 + color);
        void** p = reinterpret_cast<void**>(sp);
        p[0] = reinterpret_cast<void*>(&Nova_FiberExit);
        p[1] = reinterpret_cast<void*>(entry);
        p[2] = arg;
        p[3] = this;
        auto* c = static_cast<Prm::FiberContext*>(Prm::Heap::AllocRaw(Prm::Heap::GetProcessDefault(), sizeof(Prm::FiberContext)).ValueOr(nullptr));
        if (!c) { state = FiberState::Dead; return; }
        c->rsp = reinterpret_cast<void*>(sp);
        c->rip = reinterpret_cast<void*>(&Nova_FiberEnter);
        c->owner = this;
        c->entry = fn;
        c->arg = a;
        c->ret = static_cast<Prm::FiberContext*>(returnCtx);
        c->heavy = false;
        ctx = c;
    }

    void Fiber::StartSwitch(void* returnCtx) noexcept {
        retCtx = returnCtx;
        state = FiberState::Running;
        frameMarker = frame.Offset();
Tools::Tracy::Message("FiberEnter");
        Prm::SwapContexts(*static_cast<Prm::FiberContext*>(retCtx), *static_cast<Prm::FiberContext*>(ctx));
Tools::Tracy::Message("FiberLeave");
frame.ResetTo(frameMarker);
    }

    void Fiber::Reset(FiberStackPool& pool) noexcept {
        if (stack.base) { pool.Free(stack); }
        stack = FiberStack{};
        entry = nullptr;
        arg = nullptr;
        state = FiberState::Free;
        if (ctx) {
            auto* c = static_cast<Prm::FiberContext*>(ctx);
            c->fiberHandle = nullptr;
            c->owner = nullptr;
            c->entry = nullptr;
            c->arg = nullptr;
            c->ret = nullptr;
            (void)Prm::Heap::FreeRaw(Prm::Heap::GetProcessDefault(), c);
            ctx = nullptr;
        }
    }
}

extern "C" void Nova_FiberExitNotify(void* fiberPtr) noexcept {
    using namespace Cap;
    auto* self = static_cast<Fiber*>(fiberPtr);
    if (!self) { Prm::ThreadYield(); for(;;){} }
    Tools::Tracy::Message("FiberExitNotify");
    self->state = Cap::FiberState::Dead;
    if (self->poolRef && self->stack.base) { self->poolRef->Free(self->stack); self->stack = Cap::FiberStack{}; }
    if (self->retCtx) { Prm::JumpContext(*static_cast<Prm::FiberContext*>(self->retCtx)); }
    Prm::ThreadYield();
    for(;;){}
}
