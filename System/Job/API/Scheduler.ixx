export module System.Job:Scheduler;

import Language;
import Memory;
import Prm.System;
import Prm.Sync:LockFree;
import Prm.Ownership:Memory;
import Cap.Concurrency:Job;
import Cap.ConcurrentContainers:MPMCQueue;
import Cap.Concurrency:Fiber;
import Cap.ConcurrentContainers:Deque;
import Cap.Concurrency:Counter;
import Cap.Concurrency:Parker;

export namespace Sys {
    struct Worker;
    struct Poller { void Poll() noexcept {} };
    export extern Poller gPoller;

    export struct Scheduler {
        Atomic<bool> m_running{false};
        UInt32 m_workerCount{0};
        Worker* m_workers{nullptr};
        Cap::MPMCQueue<Cap::Job> m_global{};
        Cap::FiberStackPool m_pool{ 2u << 20u };
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
        Prm::IntrusiveLockFreeStack<IdleNode> m_idle{};
        Atomic<UInt32> m_ioPoller{0};
        [[nodiscard]] UInt32 NextTimeoutMs() noexcept;

        static Scheduler& Instance() noexcept {
            static Scheduler s{}; return s;
        }

        [[nodiscard]] Status Start(UInt32 workers) noexcept;
        [[nodiscard]] Status Stop() noexcept;
        [[nodiscard]] Status Submit(Cap::Job j) noexcept;
        [[nodiscard]] Status SubmitBatch(Cap::Job* jobs, USize count) noexcept;
        template<typename F>
        [[nodiscard]] Status Run(F&& f) noexcept {
            Cap::JobOf<std::remove_reference_t<F>> jf{ Forward<F>(f) };
            return Submit(jf.AsJob());
        }
        void ResumeFiber(Cap::Fiber* fb) noexcept;
        static Cap::Fiber* CurrentFiber() noexcept;
        static void SetCurrentFiber(Cap::Fiber* f) noexcept;
        static Cap::FrameAllocatorResource& GetFrameAllocator() noexcept;
        struct WorkerMetrics { UInt64 runNs{0}; UInt64 waitNs{0}; UInt64 stealNs{0}; UInt64 runCount{0}; UInt64 stealHit{0}; };
        [[nodiscard]] UInt32 WorkerCount() const noexcept { return m_workerCount; }
        [[nodiscard]] bool GetWorkerMetrics(UInt32 i, WorkerMetrics& out) const noexcept;
    };

    thread_local Cap::Fiber* gCurrentFiber{nullptr};

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
            auto h = Prm::Heap::GetProcessDefault();
            auto rn = Prm::Heap::AllocRaw(h, sizeof(RangePack));
            if (!rn.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
            void* mem = rn.Value();
            FnPtr fnp = +f;
            auto* pack = new (mem) RangePack{ fnp, s, e };
            Cap::Job j{};
            j.invoke = +[](void* p) noexcept {
                auto* rp = static_cast<RangePack*>(p);
                for (Index i = rp->s; i < rp->e; ++i) { rp->fn(i); }
                (void)Prm::Heap::FreeRaw(Prm::Heap::GetProcessDefault(), p);
            };
            j.arg = pack;
            auto st = RunWithCounter(j, c);
            if (!st.Ok()) return st;
        }
        WaitForCounter(c);
        return Ok(StatusDomain::System());
    }

    export Status AwaitEvent(Prm::EventHandle h) noexcept;
    export Status AwaitTimeout(UInt32 ms) noexcept;
    export Status Await(Prm::EventHandle h) noexcept;
    export Status AwaitMs(UInt32 ms) noexcept;
}
