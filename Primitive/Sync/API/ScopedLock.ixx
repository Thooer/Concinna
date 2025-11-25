export module Prm.Sync:ScopedLock;
import Prm.Element;

export namespace Prm {
    template<typename LockType>
    struct ScopedLock {
        LockType& m_lock;
        explicit ScopedLock(LockType& lock) noexcept : m_lock(lock) { m_lock.Lock(); }
        ~ScopedLock() noexcept { m_lock.Unlock(); }
        ScopedLock(const ScopedLock&) = delete;
        ScopedLock& operator=(const ScopedLock&) = delete;
    };
}

