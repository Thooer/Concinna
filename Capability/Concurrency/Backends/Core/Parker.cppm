module Concurrency;
import Language;
import Platform;
import :Parker;

namespace Concurrency {
    bool Parker::Init() noexcept {
        auto r = Platform::EventCreate(false, false);
        if (!r.IsOk()) return false;
        m_ev = r.Value();
        return true;
    }

    void Parker::Destroy() noexcept {
        if (m_ev.Get()) {
            (void)Platform::EventDestroy(m_ev);
            m_ev = Platform::EventHandle{};
        }
    }

    void Parker::Park(UInt32 timeoutMs) noexcept {
        (void)Platform::EventWait(m_ev, timeoutMs);
    }

    void Parker::Unpark() noexcept {
        (void)Platform::EventSignal(m_ev);
    }
}