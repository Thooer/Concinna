export module Prm.Sync:Fence;
import Lang.Element;
import Lang.Semantics;
import :MemoryOrder;
import <atomic>;

export namespace Prm {
    inline void CpuRelax() noexcept { CPU_RELAX(); }
    inline void ThreadFence(MemoryOrder order) noexcept { std::atomic_thread_fence(static_cast<std::memory_order>(order)); }
    inline void CompilerBarrier() noexcept { std::atomic_signal_fence(std::memory_order_seq_cst); }
}

