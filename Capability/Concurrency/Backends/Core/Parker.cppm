module Cap.Concurrency;
import Lang;
import Prm.Threading;
import :Parker;

namespace Cap {
    bool Parker::Init() noexcept {
        auto r = Prm::EventCreate(false, false);
        if (!r.IsOk()) return false;
        m_ev = r.Value().Get();
        return true;
    }

    void Parker::Destroy() noexcept {
        if (m_ev) {
            (void)Prm::EventDestroy(Prm::EventHandle{ m_ev });
            m_ev = nullptr;
        }
    }

    void Parker::Park(UInt32 timeoutMs) noexcept {
        (void)Prm::EventWait(Prm::EventHandle{ m_ev }, timeoutMs);
    }

    void Parker::Unpark() noexcept {
        (void)Prm::EventSignal(Prm::EventHandle{ m_ev });
    }
}
