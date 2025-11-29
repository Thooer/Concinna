module Cap.Concurrency;
import Lang;
import Cap.Memory;
import Prm.Ownership;
import Prm.Threading;
import :Counter;

namespace Cap {
    void Counter::Reset(UInt32 v) noexcept { m_value.Store(v, MemoryOrder::Relaxed); }

    void Counter::Add(UInt32 v) noexcept { (void)m_value.FetchAdd(v, MemoryOrder::AcqRel); }

    UInt32 Counter::Dec() noexcept {
        auto nv = m_value.FetchSub(1u, MemoryOrder::AcqRel);
        Prm::WakeByAddressAll(&m_value);
        return nv - 1u;
    }

    void Counter::SignalComplete() noexcept {
        auto nv = m_value.FetchSub(1u, MemoryOrder::AcqRel);
        Prm::WakeByAddressAll(&m_value);
    }

    UInt32 Counter::Value() const noexcept { return m_value.Load(MemoryOrder::Acquire); }

    bool Counter::RegisterWait(void* fiberPtr) noexcept {
        auto h = Prm::Heap::GetProcessDefault();
        auto rn = Prm::Heap::AllocRaw(h, sizeof(WaitNode));
        if (!rn.IsOk()) return false;
        void* mem = rn.Value();
        auto* n = new (mem) WaitNode{};
        n->fiber = fiberPtr;
        m_waiters.Push(n);
        return true;
    }

    void Counter::DrainWaiters(void(*resume)(void*)) noexcept {
        WaitNode* n = nullptr;
        while ((n = m_waiters.Pop()) != nullptr) {
            resume(n->fiber);
            auto h = Prm::Heap::GetProcessDefault();
            (void)Prm::Heap::FreeRaw(h, n);
        }
    }

    [[nodiscard]] Status WaitForZero(Counter& c, UInt32 timeoutMs) noexcept {
        UInt32 expect = c.Value();
        while (expect != 0u) {
            auto s = Prm::WaitOnAddress(&c.m_value, &expect, sizeof(expect), timeoutMs);
            if (!s.Ok()) return s;
            expect = c.Value();
        }
        return Ok(StatusDomain::System());
    }

    void NotifyCounter(Counter& c) noexcept { Prm::WakeByAddressAll(&c.m_value); }
}
