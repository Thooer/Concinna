export module Cap.Concurrency:Fiber;

import Lang;
import Cap.Memory;
import Prm.Sync;

extern "C" void Nova_FiberEnter() noexcept;
extern "C" void Nova_FiberExit() noexcept;
extern "C" void Nova_FiberExitNotify(void* fiberPtr) noexcept;

export namespace Cap {
    enum class FiberState { Free, Ready, Running, Waiting, Dead };

    struct FiberStack { void* base{nullptr}; USize size{0}; };

    struct FiberStackPool {
        void* m_base{nullptr};
        USize m_capacity{0};
        USize m_offset{0};
        USize m_committed{0};
        USize m_chunk{64u * 1024u};
        struct FreeNode { FreeNode* next; void* ptr; };
        Prm::IntrusiveLockFreeStack<FreeNode> m_free{};
        FiberStackPool(USize capacity) noexcept;
        ~FiberStackPool() noexcept;
        [[nodiscard]] FiberStack Alloc() noexcept;
        void Free(FiberStack s) noexcept;
    };

    using FiberFunc = void(*)(void*) noexcept;

    struct Fiber {
        FiberState state{FiberState::Free};
        FiberStack stack{};
        FiberFunc entry{nullptr};
        void* arg{nullptr};
        void* ctx{nullptr};
        FiberStackPool* poolRef{nullptr};
        void* retCtx{nullptr};
        void* owner{nullptr};
        UInt8 prio{0};
        Cap::FrameAllocatorResource frame{ 1u << 20u };
        USize frameMarker{0};

        void Setup(FiberStackPool& pool, FiberFunc fn, void* a, void* returnCtx) noexcept;
        void StartSwitch(void* returnCtx) noexcept;
        void Reset(FiberStackPool& pool) noexcept;
    };

}

extern "C" void Nova_FiberExitNotify(void* fiberPtr) noexcept;
