export module Prm.Sync:Cancel;
import Prm.Element;
import :Atomic;
import :MemoryOrder;

export namespace Prm {
    struct CancelToken {
        Atomic<bool>* m_state{nullptr};
        [[nodiscard]] bool IsCancelled() const noexcept { return m_state ? m_state->Load(MemoryOrder::Acquire) : false; }
    };

    struct CancelSource {
        Atomic<bool> m_cancel{false};
        void Cancel() noexcept { m_cancel.Store(true, MemoryOrder::Release); }
        [[nodiscard]] CancelToken Token() const noexcept { return CancelToken{ const_cast<Atomic<bool>*>(&m_cancel) }; }
    };
}

