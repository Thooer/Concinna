module;
module Engine.Runtime:TaskHook;

import Lang;
import Foundation.Concurrency;
import Cap.Containers;
import Cap.Memory;
import :TaskHook;

using USize;

namespace Engine {
    struct DefaultHook final : Engine::TaskSystemHook {
        Foundation::Concurrency::JobSystem m_js{};
        Foundation::Concurrency::Atomic<USize> m_nextId{1};
        Foundation::Containers::Vector<Foundation::Concurrency::Atomic<bool>*> m_ready{nullptr};
        Scoped<Cap::IAllocator, Cap::AllocatorDeleter> m_allocScoped;
        Cap::IAllocator* m_alloc{nullptr};
        struct FallbackAlloc : Cap::IAllocator {
            StatusResult<Byte*> Allocate(USize size, USize align) noexcept override {
                Byte* p = static_cast<Byte*>(::operator new[](static_cast<size_t>(size), std::nothrow));
                if (!p) return StatusResult<Byte*>::Err(Err(StatusDomain::Memory(), StatusCode::Failed));
                return StatusResult<Byte*>::Ok(p);
            }
            void Deallocate(Byte* ptr, USize, USize) noexcept override { ::operator delete[](ptr, std::nothrow); }
            StatusResult<Byte*> Reallocate(Byte* ptr, USize, USize newSize, USize) noexcept override {
                ::operator delete[](ptr, std::nothrow);
                Byte* p = static_cast<Byte*>(::operator new[](static_cast<size_t>(newSize), std::nothrow));
                if (!p) return StatusResult<Byte*>::Err(Err(StatusDomain::Memory(), StatusCode::Failed));
                return StatusResult<Byte*>::Ok(p);
            }
            Cap::AllocatorStats Stats() const noexcept override { return Cap::AllocatorStats{}; }
            bool Owns(Byte*) const noexcept override { return true; }
            UInt64 Capabilities() const noexcept override { return 0; }
        };
        FallbackAlloc* m_fallback{nullptr};

        DefaultHook() noexcept {
            auto a = Cap::CreateDefaultAllocatorScoped();
            if (a.IsOk()) { m_allocScoped = Move(a.OkValue()); m_alloc = m_allocScoped.Get(); }
            else { m_fallback = new(std::nothrow) FallbackAlloc{}; m_alloc = m_fallback; }
            m_ready = Foundation::Containers::Vector<Foundation::Concurrency::Atomic<bool>*>(m_alloc);
            (void)m_js.Start(static_cast<USize>(1));
        }
        ~DefaultHook() override {
            m_js.Stop();
            auto** data = m_ready.data();
            for (USize i=0;i<m_ready.size();++i) {
                auto* p = data[static_cast<size_t>(i)];
                if (p) {
                    p->~Atomic<bool>();
                    (void)m_alloc->Deallocate(reinterpret_cast<Byte*>(p), static_cast<USize>(sizeof(Foundation::Concurrency::Atomic<bool>)), static_cast<USize>(alignof(Foundation::Concurrency::Atomic<bool>)));
                }
            }
            if (m_fallback) { delete m_fallback; m_fallback = nullptr; }
        }

        TaskHandle Submit(void(*fn)(void*), void* user) noexcept override {
            USize id = m_nextId.FetchAdd(static_cast<USize>(1));
            ensureCapacity(id + static_cast<USize>(1));
            USize n = static_cast<USize>(m_ready.size());
            if (id >= n) return MakeTaskHandle(static_cast<USize>(0));
            auto** data = m_ready.data();
            (*data[static_cast<size_t>(id)]).Store(false);
            (void)m_js.EnqueuePriority([this, fn, user, id]() noexcept {
                if (fn) fn(user);
                auto** d = m_ready.data();
                (*d[static_cast<size_t>(id)]).Store(true);
            }, 1);
            return MakeTaskHandle(id);
        }
        bool IsReady(TaskHandle h) const noexcept override {
            USize id = h.id; if (id==0) return false;
            USize n = static_cast<USize>(m_ready.size());
            if (id >= n) return false;
            const auto* data = m_ready.data();
            return (*data[static_cast<size_t>(id)]).Load();
        }
        void Fence() noexcept override { m_js.Fence(); }

        bool Submit(std::function<void()> fn) noexcept { return m_js.EnqueuePriority(Move(fn), 1); }
        bool SubmitTo(USize workerIdx, std::function<void()> fn) noexcept { return m_js.SubmitTo(workerIdx, Move(fn)); }
        bool EnqueuePriority(FunctionView<void() noexcept> fn, int level) noexcept override {
            std::function<void()> wrap = [fn]() noexcept { fn(); };
            return m_js.EnqueuePriority(Move(wrap), level);
        }

        USize CreateJob(FunctionView<void() noexcept> fn) noexcept override {
            std::function<void()> wrap = [fn]() noexcept { fn(); };
            return m_js.CreateJob(Move(wrap), m_alloc);
        }
        bool AddEdge(USize src, USize dst) noexcept override { return m_js.AddEdge(src, dst); }
        USize MakeBarrier(USize prereqCount) noexcept override { return m_js.MakeBarrier(prereqCount, m_alloc); }
        void StartGraph() noexcept override { m_js.StartGraph(); }
        bool BuildStages() noexcept override { return m_js.BuildStages(m_alloc); }
        bool RunStages() noexcept override { return m_js.RunStages(); }

    private:
        void ensureCapacity(USize cap) noexcept {
            USize n = m_ready.size();
            if (cap <= n) return;
            USize to = cap;
            USize old = n;
            for (USize i=old;i<to;++i) {
                auto r = m_alloc->Allocate(static_cast<USize>(sizeof(Foundation::Concurrency::Atomic<bool>)), static_cast<USize>(alignof(Foundation::Concurrency::Atomic<bool>)));
                if (!r.IsOk()) break;
                auto* mem = reinterpret_cast<Byte*>(r.OkValue());
                auto* obj = new (mem) Foundation::Concurrency::Atomic<bool>{false};
                (void)m_ready.push_back(obj);
            }
        }
    };

}