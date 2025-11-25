module;
module Engine.Runtime;

import Language;
import :TaskHook;
import Foundation.Time;
import Foundation.Profiling;
import Foundation.Memory;

using Language::USize;
using Language::Int32;

namespace Engine {
    static USize findInsertIndex(const Int32* orders, USize count, Int32 order) noexcept {
        for (USize i=0;i<count;++i) { if (orders[static_cast<size_t>(i)] > order) return i; }
        return count;
    }

    bool EngineRuntime::Initialize(const CoreConfig& cfg) noexcept {
        m_cfg = cfg;
        auto a = ::Foundation::Memory::CreateDefaultAllocatorScoped();
        if (!a.IsOk()) return false;
        m_allocScoped = Language::Move(a.OkValue());
        m_alloc = m_allocScoped.Get();
        m_systems = nullptr;
        m_orders  = nullptr;
        m_count = 0;
        m_capacity = 0;
        m_prof.BeginFrame();
        m_ctx.frameStart = Foundation::Time::SteadyClock::Now();
        return true;
    }

    bool EngineRuntime::RegisterSystem(ISystem* sys, Int32 order) noexcept {
        if (!sys) return false;
        if (m_count >= m_cfg.maxSystems) return false;
        if (!sys->Initialize(m_cfg)) return false;
        if (m_count == m_capacity) {
            USize newCap = m_capacity ? m_capacity * static_cast<USize>(2) : static_cast<USize>(8);
            if (newCap > m_cfg.maxSystems) newCap = m_cfg.maxSystems;
            auto* newSystems = static_cast<ISystem**>(::operator new[](static_cast<size_t>(newCap) * sizeof(ISystem*)));
            auto* newOrders  = static_cast<Int32*>(::operator new[](static_cast<size_t>(newCap) * sizeof(Int32)));
            for (USize i=0;i<m_count;++i) {
                newSystems[static_cast<size_t>(i)] = m_systems ? m_systems[static_cast<size_t>(i)] : nullptr;
                newOrders [static_cast<size_t>(i)] = m_orders  ? m_orders [static_cast<size_t>(i)]  : 0;
            }
            if (m_systems) ::operator delete[](m_systems);
            if (m_orders)  ::operator delete[](m_orders);
            m_systems = newSystems;
            m_orders  = newOrders;
            m_capacity = newCap;
        }
        USize idx = findInsertIndex(m_orders, m_count, order);
        for (USize i=m_count;i>idx;--i) {
            m_systems[static_cast<size_t>(i)] = m_systems[static_cast<size_t>(i-1)];
            m_orders [static_cast<size_t>(i)] = m_orders [static_cast<size_t>(i-1)];
        }
        m_systems[static_cast<size_t>(idx)] = sys;
        m_orders [static_cast<size_t>(idx)] = order;
        m_count += 1;
        return true;
    }

    void EngineRuntime::BeginFrame() noexcept {
        m_prof.BeginFrame();
        m_ctx.frameStart = Foundation::Time::SteadyClock::Now();
    }

    void EngineRuntime::Tick(float dt) noexcept {
        for (USize i=0;i<m_count;++i) {
            ISystem* s = m_systems[static_cast<size_t>(i)];
            if (s) s->Tick(dt);
        }
    }

    void EngineRuntime::EndFrame() noexcept {
        if (auto* h = GetTaskSystemHook()) { h->Fence(); }
        m_ctx.frameDuration = m_prof.EndFrame();
    }

    void EngineRuntime::Shutdown() noexcept {
        for (USize i=0;i<m_count;++i) {
            ISystem* s = m_systems[static_cast<size_t>(i)];
            if (s) s->Shutdown();
        }
        if (m_systems) { ::operator delete[](m_systems); m_systems = nullptr; }
        if (m_orders)  { ::operator delete[](m_orders);  m_orders  = nullptr; }
        m_capacity = 0; m_count = 0;
        m_alloc = nullptr;
    }

    USize EngineRuntime::SystemCount() const noexcept { return m_count; }

    static TaskSystemHook* g_taskHook = nullptr;
    void SetTaskSystemHook(TaskSystemHook* h) noexcept { g_taskHook = h; }
    TaskSystemHook* GetTaskSystemHook() noexcept { return g_taskHook ? g_taskHook : static_cast<TaskSystemHook*>(nullptr); }
}
