export module Cap.Memory:ConcurrentContainers;

import Lang;
import Lang.Flow;
import Prm.Ownership;
import Prm.Sync;
 

export namespace Cap {
    template<typename Node>
    using IntrusiveLockFreeStack = Prm::IntrusiveLockFreeStack<Node>;

    using EbrRecord  = Prm::EbrRecord;
    using EbrNode    = Prm::EbrNode;
    using EbrManager = Prm::EbrManager;

    inline void* EbrHeapAlloc(USize size) noexcept { return ::operator new(size); }
    inline void EbrHeapFree(void* p) noexcept { ::operator delete(p); }
    inline void ConfigureEbrAllocatorToHeap(EbrManager& m) noexcept {
        m.SetAllocatorOps({ &EbrHeapAlloc, &EbrHeapFree });
    }
}
