module Concurrency;
import Language;
import Memory;
import Platform;
import System;
import :Job;
import :MPMCQueue;
import :Fiber;
import :Deque;
import :Counter;
import :Scheduler;
import :Parker;
import :Driver;
import Tools.Tracy;

namespace Concurrency {
    struct Worker {
        UInt32 id{0};
        ChaseLevDeque<Fiber*> qHigh{};
        ChaseLevDeque<Fiber*> qNorm{};
        Platform::ThreadHandle th{};
        Platform::FiberContext host{};
        Scheduler* sched{nullptr};
        Parker sleep{};
        Atomic<bool> idle{false};
        Scheduler::IdleNode idleNode{nullptr, this};
        UInt32 spinMax{64};
        UInt32 numaNode{0};
        Scheduler::WorkerMetrics m{};
        bool Init(Scheduler* s, UInt32 wid, void* storageH, void* storageN, USize cap) noexcept {
            id = wid; sched = s; idleNode.w = this; auto okH = qHigh.Init(storageH, cap); auto okN = qNorm.Init(storageN, cap); if (!(okH && okN)) return false; return sleep.Init();
        }
        ~Worker() noexcept { sleep.Destroy(); }
    };
    Poller gPoller{};
    bool Scheduler::GetWorkerMetrics(UInt32 i, WorkerMetrics& out) const noexcept {
        if (i >= m_workerCount) return false; const auto* w = m_workers + i; out.runNs = w->m.runNs; out.waitNs = w->m.waitNs; out.stealNs = w->m.stealNs; out.runCount = w->m.runCount; out.stealHit = w->m.stealHit; return true;
    }

    Fiber* Scheduler::CurrentFiber() noexcept { return gCurrentFiber; }
    void Scheduler::SetCurrentFiber(Fiber* f) noexcept { gCurrentFiber = f; }
    Memory::FrameAllocatorResource& Scheduler::GetFrameAllocator() noexcept { return gCurrentFiber->frame; }

    void WorkerStart(void* p) noexcept {
        auto* w = static_cast<Worker*>(p);
        Tools::Tracy::SetThreadName("Worker");
        auto& sch = *w->sched;
        UInt32 idle = 0;
        UInt32 steals = 0;
        UInt32 localProcessed = 0;
        for (;;) {
            if (!sch.m_running.Load(MemoryOrder::Acquire)) break;
            Fiber* fb = nullptr;
            if (w->qHigh.PopBottom(fb) || w->qNorm.PopBottom(fb)) {
                Scheduler::SetCurrentFiber(fb);
                auto t0 = Platform::Time::Now();
                Tools::Tracy::Zone zFiber{"Fiber"};
                fb->StartSwitch(w->host);
                auto t1 = Platform::Time::Now();
                w->m.runNs += Platform::Time::Delta(t0, t1);
                w->m.runCount += 1;
                Scheduler::SetCurrentFiber(nullptr);
                idle = 0;
                ++localProcessed;
                continue;
            }
            Job job{};
            if ((localProcessed % 64 == 0) && sch.m_global.Dequeue(job).Value()) {
                if (Platform::IsOSFiberBackend()) {
                    auto t0 = Platform::Time::Now();
                    Tools::Tracy::Zone zJob{"Job"};
                    job.invoke(job.arg);
                    auto t1 = Platform::Time::Now();
                    w->m.runNs += Platform::Time::Delta(t0, t1);
                    w->m.runCount += 1;
                    idle = 0;
                    continue;
                } else {
                    Fiber* nf = static_cast<Fiber*>(Platform::Memory::Heap::AllocRaw(Platform::Memory::Heap::GetProcessDefault(), sizeof(Fiber)).Value());
                    new (nf) Fiber{};
                    nf->Setup(sch.m_pool, job.invoke, job.arg, w->host);
                    nf->owner = w;
                    nf->prio = job.qos;
                    Scheduler::SetCurrentFiber(nf);
                    auto t0 = Platform::Time::Now();
                    Tools::Tracy::Zone zFiberJob{"Fiber(Job)"};
                    nf->StartSwitch(w->host);
                    auto t1 = Platform::Time::Now();
                    w->m.runNs += Platform::Time::Delta(t0, t1);
                    w->m.runCount += 1;
                    Scheduler::SetCurrentFiber(nullptr);
                    idle = 0;
                    continue;
                }
            }
            Fiber* stolen = nullptr;
            auto ts0 = Platform::Time::Now();
            UInt32 gs = sch.m_groupSize ? sch.m_groupSize : 1;
            UInt32 base = (w->id / gs) * gs;
            for (UInt32 i = 0; i < gs; ++i) {
                auto idx = base + ((w->id + 1 + i) % gs);
                auto* v = sch.m_workers + idx;
                if (v != w && (v->qHigh.Steal(stolen) || v->qNorm.Steal(stolen))) { break; }
            }
            if (!stolen && sch.m_l3Members && sch.m_l3Counts) {
                auto node = w->numaNode;
                auto grp = sch.m_l3OfWorker ? sch.m_l3OfWorker[w->id] : 0u;
                auto off = sch.m_l3Offsets ? sch.m_l3Offsets[grp] : 0u;
                auto cnt = sch.m_l3Counts[grp];
                for (UInt32 i = 0; i < cnt; ++i) {
                    auto wid = sch.m_l3Members[off + i];
                    auto* v = sch.m_workers + wid;
                    if (v != w && (v->qHigh.Steal(stolen) || v->qNorm.Steal(stolen))) { break; }
                }
            }
            if (!stolen && sch.m_numaMembers && sch.m_numaCounts) {
                auto node = w->numaNode;
                auto off = sch.m_numaOffsets ? sch.m_numaOffsets[node] : 0u;
                auto cnt = sch.m_numaCounts[node];
                for (UInt32 i = 0; i < cnt; ++i) {
                    auto wid = sch.m_numaMembers[off + i];
                    auto* v = sch.m_workers + wid;
                    if (v != w && (v->qHigh.Steal(stolen) || v->qNorm.Steal(stolen))) { break; }
                }
            }
            if (!stolen) {
                for (UInt32 i = 0; i < sch.m_workerCount; ++i) {
                    auto* v = sch.m_workers + ((w->id + 1 + i) % sch.m_workerCount);
                    if (v != w && (v->qHigh.Steal(stolen) || v->qNorm.Steal(stolen))) { break; }
                }
            }
            auto ts1 = Platform::Time::Now();
            w->m.stealNs += Platform::Time::Delta(ts0, ts1);
            if (stolen) {
                Scheduler::SetCurrentFiber(stolen);
                auto t0 = Platform::Time::Now();
                Tools::Tracy::Zone zStolen{"Fiber(Stolen)"};
                stolen->StartSwitch(w->host);
                auto t1 = Platform::Time::Now();
                w->m.runNs += Platform::Time::Delta(t0, t1);
                w->m.runCount += 1;
                w->m.stealHit += 1;
                Scheduler::SetCurrentFiber(nullptr);
                idle = 0;
                ++steals;
                continue;
            }
            if (sch.m_ioPoller.Load(MemoryOrder::Relaxed) == w->id) { gDriverApi->Poll(); }
            ++idle;
            if (idle < w->spinMax) {
                Platform::ThreadYield();
            } else {
                if (!w->idle.Load(MemoryOrder::Relaxed)) {
                    bool expected = false;
                    if (w->idle.CompareExchangeStrong(expected, true, MemoryOrder::AcqRel, MemoryOrder::Relaxed)) {
                        sch.m_idle.Push(&w->idleNode);
                    }
                }
                auto t0 = Platform::Time::Now();
                w->sleep.Park(sch.NextTimeoutMs());
                auto t1 = Platform::Time::Now();
                auto dt = Platform::Time::Delta(t0, t1);
                w->m.waitNs += dt;
                w->idle.Store(false, MemoryOrder::Release);
                if (dt < 2'000) { w->spinMax = w->spinMax < 256 ? (w->spinMax << 1) : 256; }
                else { w->spinMax = w->spinMax > 32 ? (w->spinMax >> 1) : 32; }
                idle = 0;
                if (sch.m_workerCount > 1 && sch.m_ioPoller.Load(MemoryOrder::Relaxed) == w->id) {
                    auto nxt = (w->id + 1) % sch.m_workerCount;
                    sch.m_ioPoller.Store(nxt, MemoryOrder::Relaxed);
                }
            }
        }
    }

    Status Scheduler::Start(UInt32 workers) noexcept {
        if (m_running.Load(MemoryOrder::Relaxed)) return Ok(StatusDomain::System());
        if (workers == 0) {
            auto c = System::SystemInfo::Cpu(); workers = c.physicalCores ? c.physicalCores : 1;
        }
        auto h = Platform::Memory::Heap::GetProcessDefault();
        auto arr = Platform::Memory::Heap::AllocRaw(h, sizeof(Worker) * workers);
        if (!arr.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
        m_workers = static_cast<Worker*>(arr.Value());
        m_workerCount = workers;
        {
            auto topo = System::Detect();
            m_groupSize = topo.threadsPerCore ? topo.threadsPerCore : 1u;
        }
        for (UInt32 i = 0; i < workers; ++i) { new (m_workers + i) Worker{}; }
        const USize cap = 1024;
        for (UInt32 i = 0; i < workers; ++i) {
            auto sbh = Platform::Memory::Heap::AllocRaw(h, sizeof(Fiber*) * cap);
            if (!sbh.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
            auto sbn = Platform::Memory::Heap::AllocRaw(h, sizeof(Fiber*) * cap);
            if (!sbn.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
            if (!m_workers[i].Init(this, i, sbh.Value(), sbn.Value(), cap)) return Err(StatusDomain::System(), StatusCode::Failed);
        }
        {
            auto cms = System::EnumerateCoreMasks();
            auto nms = System::EnumerateNumaNodeMasks();
            if (cms.data && cms.count > 0) {
                auto h2 = Platform::Memory::Heap::GetProcessDefault();
                auto ra = Platform::Memory::Heap::AllocRaw(h2, sizeof(UInt32) * m_workerCount);
                if (!ra.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
                m_numaOfWorker = static_cast<UInt32*>(ra.Value());
                for (UInt32 i = 0; i < m_workerCount; ++i) { m_numaOfWorker[i] = 0u; }
                UInt32 gs = m_groupSize ? m_groupSize : 1u;
                for (UInt32 i = 0; i < m_workerCount; ++i) {
                    UInt32 ci = i / gs;
                    UInt32 li = i % gs;
                    UInt64 bit = 0u;
                    UInt32 grp = 0u;
                    if (ci < cms.count) {
                        grp = cms.data[ci].group;
                        UInt64 m = cms.data[ci].mask;
                        UInt32 k = 0u;
                        for (UInt32 b = 0; b < 64u; ++b) {
                            if (m & (UInt64(1) << b)) { if (k == li) { bit = (UInt64(1) << b); break; } ++k; }
                        }
                    }
                    UInt32 nodeSel = 0u;
                    if (nms.data && nms.count > 0) {
                        for (USize j = 0; j < nms.count; ++j) {
                            if (nms.data[j].group == grp && (bit != 0u) && (nms.data[j].mask & bit)) { nodeSel = nms.data[j].node; break; }
                        }
                    }
                    m_numaOfWorker[i] = nodeSel;
                    m_workers[i].numaNode = nodeSel;
                }
                UInt32 nodeMax = 0u; for (USize j = 0; j < nms.count; ++j) { if (nms.data[j].node > nodeMax) nodeMax = nms.data[j].node; }
                m_numaNodeCount = (nms.count > 0) ? (nodeMax + 1u) : 1u;
                auto rc = Platform::Memory::Heap::AllocRaw(h2, sizeof(UInt32) * m_numaNodeCount);
                if (!rc.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
                m_numaCounts = static_cast<UInt32*>(rc.Value());
                for (UInt32 i = 0; i < m_numaNodeCount; ++i) { m_numaCounts[i] = 0u; }
                for (UInt32 i = 0; i < m_workerCount; ++i) { ++m_numaCounts[m_numaOfWorker[i]]; }
                auto rof = Platform::Memory::Heap::AllocRaw(h2, sizeof(UInt32) * m_numaNodeCount);
                if (!rof.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
                m_numaOffsets = static_cast<UInt32*>(rof.Value());
                UInt32 acc = 0u; for (UInt32 i = 0; i < m_numaNodeCount; ++i) { m_numaOffsets[i] = acc; acc += m_numaCounts[i]; }
                auto rmm = Platform::Memory::Heap::AllocRaw(h2, sizeof(UInt32) * m_workerCount);
                if (!rmm.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
                m_numaMembers = static_cast<UInt32*>(rmm.Value());
                for (UInt32 i = 0; i < m_numaNodeCount; ++i) { m_numaCounts[i] = 0u; }
                for (UInt32 i = 0; i < m_workerCount; ++i) {
                    auto node = m_numaOfWorker[i]; auto off = m_numaOffsets[node]; auto pos = off + m_numaCounts[node]++;
                    m_numaMembers[pos] = i;
                }
            }
            System::Release(cms);
            System::Release(nms);
        }
        {
            auto cms = System::EnumerateCoreMasks();
            auto caches = System::EnumerateCacheMasks();
            if (caches.data && caches.count > 0) {
                auto h2 = Platform::Memory::Heap::GetProcessDefault();
                auto ro = Platform::Memory::Heap::AllocRaw(h2, sizeof(UInt32) * m_workerCount);
                if (!ro.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
                m_l3OfWorker = static_cast<UInt32*>(ro.Value());
                for (UInt32 i = 0; i < m_workerCount; ++i) { m_l3OfWorker[i] = 0u; }
                UInt32 gs = m_groupSize ? m_groupSize : 1u;
                for (UInt32 i = 0; i < m_workerCount; ++i) {
                    UInt32 ci = i / gs; UInt32 li = i % gs;
                    UInt64 bit = 0u; UInt32 grp = 0u;
                    if (cms.data && ci < cms.count) {
                        grp = cms.data[ci].group; UInt64 m = cms.data[ci].mask; UInt32 k = 0u;
                        for (UInt32 b = 0; b < 64u; ++b) { if (m & (UInt64(1) << b)) { if (k == li) { bit = (UInt64(1) << b); break; } ++k; } }
                    }
                    UInt32 l3id = 0u;
                    for (USize j = 0; j < caches.count; ++j) {
                        if (caches.data[j].level == 3u && caches.data[j].group == grp) {
                            if (bit != 0u && (caches.data[j].mask & bit)) { l3id = caches.data[j].id; break; }
                        }
                    }
                    m_l3OfWorker[i] = l3id;
                }
                UInt32 maxId = 0u; for (UInt32 i = 0; i < m_workerCount; ++i) { if (m_l3OfWorker[i] > maxId) maxId = m_l3OfWorker[i]; }
                m_l3GroupCount = maxId + 1u;
                auto rc = Platform::Memory::Heap::AllocRaw(h2, sizeof(UInt32) * m_l3GroupCount);
                if (!rc.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
                m_l3Counts = static_cast<UInt32*>(rc.Value());
                for (UInt32 i = 0; i < m_l3GroupCount; ++i) { m_l3Counts[i] = 0u; }
                for (UInt32 i = 0; i < m_workerCount; ++i) { ++m_l3Counts[m_l3OfWorker[i]]; }
                auto rof = Platform::Memory::Heap::AllocRaw(h2, sizeof(UInt32) * m_l3GroupCount);
                if (!rof.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
                m_l3Offsets = static_cast<UInt32*>(rof.Value());
                UInt32 acc = 0u; for (UInt32 i = 0; i < m_l3GroupCount; ++i) { m_l3Offsets[i] = acc; acc += m_l3Counts[i]; }
                auto rm = Platform::Memory::Heap::AllocRaw(h2, sizeof(UInt32) * m_workerCount);
                if (!rm.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
                m_l3Members = static_cast<UInt32*>(rm.Value());
                for (UInt32 i = 0; i < m_l3GroupCount; ++i) { m_l3Counts[i] = 0u; }
                for (UInt32 i = 0; i < m_workerCount; ++i) {
                    auto grp = m_l3OfWorker[i]; auto off = m_l3Offsets[grp]; auto pos = off + m_l3Counts[grp]++;
                    m_l3Members[pos] = i;
                }
                System::Release(cms);
                System::Release(caches);
            }
        }
        (void)gDriverApi->Init();
        m_running.Store(true, MemoryOrder::Release);
        m_ioPoller.Store(workers ? (workers - 1u) : 0u, MemoryOrder::Relaxed);
        for (UInt32 i = 0; i < workers; ++i) {
            auto th = Platform::ThreadCreate(&WorkerStart, m_workers + i);
            if (!th.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
            m_workers[i].th = th.Value();
        }
        return Ok(StatusDomain::System());
    }

    Status Scheduler::Stop() noexcept {
        m_running.Store(false, MemoryOrder::Release);
        for (UInt32 i = 0; i < m_workerCount; ++i) {
            (void)Platform::ThreadJoin(m_workers[i].th);
        }
        auto h = Platform::Memory::Heap::GetProcessDefault();
        for (UInt32 i = 0; i < m_workerCount; ++i) { m_workers[i].~Worker(); }
        if (m_numaMembers) { (void)Platform::Memory::Heap::FreeRaw(h, m_numaMembers); m_numaMembers = nullptr; }
        if (m_numaOffsets) { (void)Platform::Memory::Heap::FreeRaw(h, m_numaOffsets); m_numaOffsets = nullptr; }
        if (m_numaCounts) { (void)Platform::Memory::Heap::FreeRaw(h, m_numaCounts); m_numaCounts = nullptr; }
        if (m_numaOfWorker) { (void)Platform::Memory::Heap::FreeRaw(h, m_numaOfWorker); m_numaOfWorker = nullptr; }
        if (m_l3Members) { (void)Platform::Memory::Heap::FreeRaw(h, m_l3Members); m_l3Members = nullptr; }
        if (m_l3Offsets) { (void)Platform::Memory::Heap::FreeRaw(h, m_l3Offsets); m_l3Offsets = nullptr; }
        if (m_l3Counts) { (void)Platform::Memory::Heap::FreeRaw(h, m_l3Counts); m_l3Counts = nullptr; }
        if (m_l3OfWorker) { (void)Platform::Memory::Heap::FreeRaw(h, m_l3OfWorker); m_l3OfWorker = nullptr; }
        m_workerCount = 0;
        gDriverApi->Shutdown();
        return Ok(StatusDomain::System());
    }

    Status Scheduler::Submit(Job j) noexcept {
        auto r = m_global.Enqueue(j);
        if (!r.IsOk()) return r.Error();
        auto wakeSome = +[](Scheduler& sch, UInt32 want) noexcept {
            Scheduler::IdleNode* stash[16];
            UInt32 sc = 0u;
            UInt32 woke = 0u;
            for (UInt32 i = 0u; i < 16u && woke < want; ++i) {
                auto* n = sch.m_idle.Pop();
                if (!n) break;
                auto* w = n->w;
                if (w && w->idle.Load(MemoryOrder::Relaxed)) { w->sleep.Unpark(); ++woke; }
                else { if (sc < 16u) { stash[sc++] = n; } }
            }
            while (sc > 0u) { sch.m_idle.Push(stash[--sc]); }
        };
        wakeSome(*this, 1u);
        return Ok(StatusDomain::System());
    }

    void Scheduler::ResumeFiber(Fiber* fb) noexcept {
        if (!fb) return;
        auto* w = static_cast<Worker*>(fb->owner);
        if (!w) return;
        if (fb->prio == static_cast<UInt8>(QoS::High)) { (void)w->qHigh.PushBottom(fb); }
        else { (void)w->qNorm.PushBottom(fb); }
        auto wakeSome = +[](Scheduler& sch, UInt32 want) noexcept {
            Scheduler::IdleNode* stash[16];
            UInt32 sc = 0u;
            UInt32 woke = 0u;
            for (UInt32 i = 0u; i < 16u && woke < want; ++i) {
                auto* n = sch.m_idle.Pop();
                if (!n) break;
                auto* w = n->w;
                if (w && w->idle.Load(MemoryOrder::Relaxed)) { w->sleep.Unpark(); ++woke; }
                else { if (sc < 16u) { stash[sc++] = n; } }
            }
            while (sc > 0u) { sch.m_idle.Push(stash[--sc]); }
        };
        wakeSome(*this, 1u);
    }

    Status Scheduler::SubmitBatch(Job* jobs, USize count) noexcept {
        if (!jobs || count == 0) return Ok(StatusDomain::System());
        auto r = m_global.EnqueueBatch(jobs, count);
        if (!r.IsOk()) return r.Error();
        auto wakeSome = +[](Scheduler& sch, UInt32 want) noexcept {
            Scheduler::IdleNode* stash[32];
            UInt32 sc = 0u;
            UInt32 woke = 0u;
            for (UInt32 i = 0u; i < 32u && woke < want; ++i) {
                auto* n = sch.m_idle.Pop();
                if (!n) break;
                auto* w = n->w;
                if (w && w->idle.Load(MemoryOrder::Relaxed)) { w->sleep.Unpark(); ++woke; }
                else { if (sc < 32u) { stash[sc++] = n; } }
            }
            while (sc > 0u) { sch.m_idle.Push(stash[--sc]); }
        };
        UInt32 want = static_cast<UInt32>(count);
        if (want > m_workerCount) want = m_workerCount;
        wakeSome(*this, want);
        return Ok(StatusDomain::System());
    }

    UInt32 Scheduler::NextTimeoutMs() noexcept {
        return gDriverApi->NextTimeoutMs();
    }

    static void ResumeFiberCallback(void* p) noexcept {
        auto* f = static_cast<Fiber*>(p);
        Scheduler::Instance().ResumeFiber(f);
    }

    void ResumeWaiters(Counter& c) noexcept { c.DrainWaiters(&ResumeFiberCallback); }

    void WaitForCounter(Counter& c) noexcept {
        auto* cf = Scheduler::CurrentFiber();
        if (!cf) {
            while (c.Value() != 0u) { (void)WaitForZero(c, 1); }
            return;
        }
        if (c.Value() == 0u) return;
        if (!c.RegisterWait(cf)) return;
        cf->state = FiberState::Waiting;
        Platform::SwapContexts(cf->ctx, *cf->retCtx);
        cf->state = FiberState::Running;
    }

    Status AwaitEvent(Platform::EventHandle h) noexcept {
        auto* cf = Scheduler::CurrentFiber();
        if (!cf) {
            for (;;) { auto r = Platform::EventWait(h, 1u); if (r.Ok()) break; Platform::ThreadYield(); }
            return Ok(StatusDomain::System());
        }
        if (!gDriverApi->AddEvent(h, cf)) {
            return Err(StatusDomain::System(), StatusCode::Failed);
        }
        cf->state = FiberState::Waiting;
        Platform::SwapContexts(cf->ctx, *cf->retCtx);
        cf->state = FiberState::Running;
        return Ok(StatusDomain::System());
    }

    Status AwaitTimeout(UInt32 ms) noexcept {
        SleepMs(ms);
        return Ok(StatusDomain::System());
    }

    Status Await(Platform::EventHandle h) noexcept { return AwaitEvent(h); }
    Status AwaitMs(UInt32 ms) noexcept { return AwaitTimeout(ms); }

    Status RunWithCounter(Job j, Counter& c) noexcept {
        auto& sch = Scheduler::Instance();
        struct FnPack { void(*fn)(void*) noexcept; void* arg; Counter* cc; };
        auto real = j.invoke;
        auto realArg = j.arg;
        j.invoke = +[](void* p) noexcept {
            auto* pack = static_cast<FnPack*>(p);
            pack->fn(pack->arg);
            pack->cc->SignalComplete();
            (void)Platform::Memory::Heap::FreeRaw(Platform::Memory::Heap::GetProcessDefault(), p);
        };
        auto h = Platform::Memory::Heap::GetProcessDefault();
        auto rn = Platform::Memory::Heap::AllocRaw(h, sizeof(FnPack));
        if (!rn.IsOk()) return Err(StatusDomain::System(), StatusCode::Failed);
        void* mem = rn.Value();
        auto* pack = new (mem) FnPack{ real, realArg, &c };
        Job jj{}; jj.invoke = reinterpret_cast<void(*)(void*) noexcept>(j.invoke); jj.arg = pack;
        jj.qos = j.qos;
        return sch.Submit(jj);
    }

    Status RunWithCounterPriority(Job j, Counter& c, QoS qos) noexcept {
        j.qos = static_cast<UInt8>(qos);
        return RunWithCounter(j, c);
    }

    Status SubmitPriority(Job j, QoS qos) noexcept {
        j.qos = static_cast<UInt8>(qos);
        return Scheduler::Instance().Submit(j);
    }

    void Suspend(void(*reg)(Fiber*, void*), void* ctx) noexcept {
        auto* cf = Scheduler::CurrentFiber();
        if (!cf) return;
        cf->state = FiberState::Waiting;
        if (reg) reg(cf, ctx);
        Platform::SwapContexts(cf->ctx, *cf->retCtx);
        cf->state = FiberState::Running;
    }
}