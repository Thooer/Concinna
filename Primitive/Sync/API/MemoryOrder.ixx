export module Prm.Sync:MemoryOrder;
import Lang.Element;
import <atomic>;

export namespace Prm {
    enum class MemoryOrder : Int32 {
        Relaxed = (Int32)std::memory_order_relaxed,
        Consume = (Int32)std::memory_order_consume,
        Acquire = (Int32)std::memory_order_acquire,
        Release = (Int32)std::memory_order_release,
        AcqRel  = (Int32)std::memory_order_acq_rel,
        SeqCst  = (Int32)std::memory_order_seq_cst
    };
}

