export module Sys.Job:Scheduler;

import Lang;
import Cap.Memory;
import Prm.System;
import Prm.Sync;
import Prm.Ownership;
import Cap.Concurrency;
import Cap.ConcurrentContainers;

export namespace Sys {
    using Job = Cap::Job;
    using Counter = Cap::Counter;
    using QoS = Cap::QoS;
    using Fiber = Cap::Fiber;
    struct Worker;
    struct Poller { void Poll() noexcept {} };
    export extern Poller gPoller;

    export struct Scheduler {
        Prm::Atomic<bool> m_running{false};
        UInt32 m_workerCount{0};
        Worker* m_workers{nullptr};
        Cap::MPMCQueue<Job> m_global{};
        Cap::FiberStackPool m_pool{ 2u << 20u };
        Prm::Atomic<UInt32> m_workCounter{0};
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
        Prm::IntrusiveLockFreeStack<IdleNode> m_idle{};
        Prm::Atomic<UInt32> m_ioPoller{0};
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
            Cap::JobOf<std::remove_reference_t<F>> jf{ Forward<F>(f) };
            return Submit(jf.AsJob());
        }
        void ResumeFiber(Fiber* fb) noexcept;
        static Fiber* CurrentFiber() noexcept;
        static void SetCurrentFiber(Fiber* f) noexcept;
        static Cap::FrameAllocatorResource& GetFrameAllocator() noexcept;
        struct WorkerMetrics { UInt64 runNs{0}; UInt64 waitNs{0}; UInt64 stealNs{0}; UInt64 runCount{0}; UInt64 stealHit{0}; };
        [[nodiscard]] UInt32 WorkerCount() const noexcept { return m_workerCount; }
        [[nodiscard]] bool GetWorkerMetrics(UInt32 i, WorkerMetrics& out) const noexcept;
    };

    thread_local Fiber* gCurrentFiber{nullptr};

    void WorkerStart(void* p) noexcept;

    export void ResumeWaiters(Cap::Counter& c) noexcept;
    export void WaitForCounter(Cap::Counter& c) noexcept;
    export Status RunWithCounter(Cap::Job j, Cap::Counter& c) noexcept;
    export Status RunWithCounterPriority(Cap::Job j, Cap::Counter& c, Cap::QoS qos) noexcept;
    export Status SubmitPriority(Cap::Job j, Cap::QoS qos) noexcept;
    export void Suspend(void(*reg)(Cap::Fiber*, void*), void* ctx) noexcept;

    

    export template<typename Index, typename Fn>
    Status ParallelFor(Index begin, Index end, Index grain, Fn&& f) noexcept {
        if (end <= begin || grain <= 0) return Ok(StatusDomain::System());
        auto& sch = Scheduler::Instance();
        auto total = static_cast<USize>(end - begin);
        auto tasks = static_cast<USize>((total + grain - 1) / grain);
        Cap::Counter c{}; c.Reset(static_cast<UInt32>(tasks));
        for (Index s = begin; s < end; s += grain) {
            Index e = s + grain; if (e > end) e = end;
            using FnPtr = void(*)(Index) noexcept;
            struct RangePack { FnPtr fn; Index s; Index e; };
            void* mem = ::operator new(sizeof(RangePack));
            if (!mem) return Err(StatusDomain::System(), StatusCode::Failed);
            FnPtr fnp = +f;
            auto* pack = new (mem) RangePack{ fnp, s, e };
            Cap::Job j{};
            j.invoke = +[](void* p) noexcept {
                auto* rp = static_cast<RangePack*>(p);
                for (Index i = rp->s; i < rp->e; ++i) { rp->fn(i); }
                ::operator delete(p);
            };
            j.arg = pack;
            auto st = RunWithCounter(j, c);
            if (!st.Ok()) return st;
        }
        WaitForCounter(c);
        return Ok(StatusDomain::System());
    }

    export Status AwaitEvent(void* h) noexcept;
    export Status AwaitTimeout(UInt32 ms) noexcept;
    export Status Await(void* h) noexcept;
    export Status AwaitMs(UInt32 ms) noexcept;
}
