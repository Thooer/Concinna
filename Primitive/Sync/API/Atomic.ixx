export module Prm.Sync:Atomic;
import Element;
import :MemoryOrder;
import <atomic>;

export namespace Prm {
    template<typename T>
        requires TriviallyCopyable<T>
    struct Atomic {
        using NativeType = std::atomic<T>;
        NativeType m_val;

        constexpr Atomic() noexcept = default;
        constexpr Atomic(T val) noexcept : m_val(val) {}

        Atomic(const Atomic&) = delete;
        Atomic& operator=(const Atomic&) = delete;

        [[nodiscard]] T Load(MemoryOrder order = MemoryOrder::SeqCst) const noexcept {
            return m_val.load(static_cast<std::memory_order>(order));
        }
        [[nodiscard]] T load(MemoryOrder order = MemoryOrder::SeqCst) const noexcept { return Load(order); }

        void Store(T val, MemoryOrder order = MemoryOrder::SeqCst) noexcept {
            m_val.store(val, static_cast<std::memory_order>(order));
        }
        void store(T val, MemoryOrder order = MemoryOrder::SeqCst) noexcept { Store(val, order); }

        T Exchange(T val, MemoryOrder order = MemoryOrder::SeqCst) noexcept {
            return m_val.exchange(val, static_cast<std::memory_order>(order));
        }
        T exchange(T val, MemoryOrder order = MemoryOrder::SeqCst) noexcept { return Exchange(val, order); }

        bool CompareExchangeStrong(T& expected, T desired,
                                   MemoryOrder success = MemoryOrder::SeqCst,
                                   MemoryOrder failure = MemoryOrder::SeqCst) noexcept {
            return m_val.compare_exchange_strong(expected, desired,
                                                 static_cast<std::memory_order>(success),
                                                 static_cast<std::memory_order>(failure));
        }
        bool compare_exchange_strong(T& expected, T desired,
                                     MemoryOrder success = MemoryOrder::SeqCst,
                                     MemoryOrder failure = MemoryOrder::SeqCst) noexcept {
            return CompareExchangeStrong(expected, desired, success, failure);
        }

        bool CompareExchangeWeak(T& expected, T desired,
                                 MemoryOrder success = MemoryOrder::SeqCst,
                                 MemoryOrder failure = MemoryOrder::SeqCst) noexcept {
            return m_val.compare_exchange_weak(expected, desired,
                                               static_cast<std::memory_order>(success),
                                               static_cast<std::memory_order>(failure));
        }
        bool compare_exchange_weak(T& expected, T desired,
                                   MemoryOrder success = MemoryOrder::SeqCst,
                                   MemoryOrder failure = MemoryOrder::SeqCst) noexcept {
            return CompareExchangeWeak(expected, desired, success, failure);
        }

        T FetchAdd(T arg, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires Integral<T> {
            return m_val.fetch_add(arg, static_cast<std::memory_order>(order));
        }
        T fetch_add(T arg, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires Integral<T> { return FetchAdd(arg, order); }

        T FetchSub(T arg, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires Integral<T> {
            return m_val.fetch_sub(arg, static_cast<std::memory_order>(order));
        }
        T fetch_sub(T arg, MemoryOrder order = MemoryOrder::SeqCst) noexcept requires Integral<T> { return FetchSub(arg, order); }

        T operator++() noexcept requires Integral<T> { return FetchAdd(1) + 1; }
        T operator--() noexcept requires Integral<T> { return FetchSub(1) - 1; }
    };
}

