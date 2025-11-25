export module Concurrency:Scheduler;

import Language;
import Memory;
import Platform;
import System;
import :Job;
import :MPMCQueue;
import :Fiber;
import :Deque;
import :Counter;
import :Parker;

namespace Concurrency {
    struct Worker;
    struct Poller { void Poll() noexcept {} };
    export extern Poller gPoller;

    export struct Scheduler {
        Atomic<bool> m_running{false};
        UInt32 m_workerCount{0};
        Worker* m_workers{nullptr};
        Concurrency::MPMCQueue<Job> m_global{};
        FiberStackPool m_pool{ 2u << 20u };
        Atomic<UInt32> m_workCounter{0};
        UInt32 m_groupSize{1};
        UInt32 m_numaNodeCount{0};
        UInt32* m_numaOfWorker{nullptr};
        UInt32* m_numaOffsets{nullptr};
        UInt32* m_numaCounts{nullptr};
        UInt32* m_numaMembers{nullptr};
        UInt32 m_l3GroupCount{0};
        UInt32* m_l3OfWorker{nullptr};
        UInt32* m_l3Offsets{nullptr};
        UInt32* m_l3Counts{nullptr};
        UInt32* m_l3Members{nullptr};
        struct IdleNode { IdleNode* next; Worker* w; };
        Memory::IntrusiveLockFreeStack<IdleNode> m_idle{};
        Atomic<UInt32> m_ioPoller{0};
        [[nodiscard]] UInt32 NextTimeoutMs() noexcept;

        static Scheduler& Instance() noexcept {
            static Scheduler s{}; return s;
        }

        [[nodiscard]] Status Start(UInt32 workers) noexcept;
        [[nodiscard]] Status Stop() noexcept;
        [[nodiscard]] Status Submit(Job j) noexcept;
        [[nodiscard]] Status SubmitBatch(Job* jobs, USize count) noexcept;
        template<typename F>
        [[nodiscard]] Status Run(F&& f) noexcept {
            JobOf<std::remove_reference_t<F>> jf{ Forward<F>(f) };
            return Submit(jf.AsJob());
        }
        void ResumeFiber(Fiber* fb) noexcept;
        static Fiber* CurrentFiber() noexcept;
        static void SetCurrentFiber(Fiber* f) noexcept;
        static Memory::FrameAllocatorResource& GetFrameAllocator() noexcept;
        struct WorkerMetrics { UInt64 runNs{0}; UInt64 waitNs{0}; UInt64 stealNs{0}; UInt64 runCount{0}; UInt64 stealHit{0}; };
        [[nodiscard]] UInt32 WorkerCount() const noexcept { return m_workerCount; }
        [[nodiscard]] bool GetWorkerMetrics(UInt32 i, WorkerMetrics& out) const noexcept;
    };

    thread_local Fiber* gCurrentFiber{nullptr};


    void WorkerStart(void* p) noexcept;

    

    export void ResumeWaiters(Counter& c) noexcept;

    export void WaitForCounter(Counter& c) noexcept;

    export Status RunWithCounter(Job j, Counter& c) noexcept;

    export Status RunWithCounterPriority(Job j, Counter& c, QoS qos) noexcept;

    export Status SubmitPriority(Job j, QoS qos) noexcept;

    export void Suspend(void(*reg)(Fiber*, void*), void* ctx) noexcept;

    export template<typename Index, typename Fn>
    Status ParallelFor(Index begin, Index end, Index grain, Fn&& f) noexcept {
        if (end <= begin || grain <= 0) return Ok(StatusDomain::System());
        auto& sch = Scheduler::Instance();
        auto total = static_cast<USize>(end - begin);
        auto tasks = static_cast<USize>((total + grain - 1) / grain);
        Counter c{}; c.Reset(static_cast<UInt32>(tasks));
        for (Index s = begin; s < end; s += grain) {
            Index e = s + grain; if (e > end) e = end;
            using FnPtr = void(*)(Index) noexcept;
            struct RangePack { FnPtr fn; Index s; Index e; };
            auto h = Platform::Memory::Heap::GetProcessDefault();
            auto rn = Platform::Memory::Heap::AllocRaw(h, sizeof(RangePack));
            if (!rn.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
            void* mem = rn.Value();
            FnPtr fnp = +f;
            auto* pack = new (mem) RangePack{ fnp, s, e };
            Job j{};
            j.invoke = +[](void* p) noexcept {
                auto* rp = static_cast<RangePack*>(p);
                for (Index i = rp->s; i < rp->e; ++i) { rp->fn(i); }
                // Counter dec and resume handled by RunWithCounter wrapper
                (void)Platform::Memory::Heap::FreeRaw(Platform::Memory::Heap::GetProcessDefault(), p);
            };
            j.arg = pack;
            auto st = RunWithCounter(j, c);
            if (!st.Ok()) return st;
        }
        WaitForCounter(c);
        return Ok(StatusDomain::System());
    }
    export Status AwaitEvent(Platform::EventHandle h) noexcept;
    export Status AwaitTimeout(UInt32 ms) noexcept;
    export Status Await(Platform::EventHandle h) noexcept;
    export Status AwaitMs(UInt32 ms) noexcept;
}
