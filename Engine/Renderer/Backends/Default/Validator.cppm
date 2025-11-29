module;
module Engine.Renderer;

import Lang;
import :Validator;
import :FrameGraph;
import Foundation.Containers;

namespace Engine::Renderer {
    FrameGraphValidator::FrameGraphValidator(::Foundation::Memory::IAllocator* alloc) noexcept : m_alloc(alloc), m_report{} {}
    bool FrameGraphValidator::CheckConflicts(const FrameGraph& fg) noexcept {
        auto up = fg.UsagePass();
        auto ur = fg.UsageRes();
        auto um = fg.UsageMode();
        auto order = fg.Order();
        Language::USize n = static_cast<Language::USize>(order.size());
        ::Foundation::Containers::Vector<Language::UInt32> topoIndex(m_alloc);
        (void)topoIndex.resize(n);
        for (Language::USize i = 0; i < n; ++i) topoIndex.data()[static_cast<size_t>(order.data()[static_cast<size_t>(i)])] = static_cast<Language::UInt32>(i);

        Language::USize rc = fg.ResourceCount();
        ::Foundation::Containers::Vector<Language::UInt32> firstMode(m_alloc);
        ::Foundation::Containers::Vector<Language::UInt32> lastPass(m_alloc);
        (void)firstMode.resize(rc);
        (void)lastPass.resize(rc);
        for (Language::USize i = 0; i < rc; ++i) { firstMode.data()[static_cast<size_t>(i)] = static_cast<Language::UInt32>(~0u); lastPass.data()[static_cast<size_t>(i)] = 0; }

        for (Language::USize i = 0; i < up.size(); ++i) {
            Language::UInt32 r = ur.data()[static_cast<size_t>(i)];
            Language::UInt32 p = up.data()[static_cast<size_t>(i)];
            Language::UInt32 m = um.data()[static_cast<size_t>(i)];
            Language::UInt32 ti = topoIndex.data()[static_cast<size_t>(p)];
            if (firstMode.data()[static_cast<size_t>(r)] == static_cast<Language::UInt32>(~0u)) firstMode.data()[static_cast<size_t>(r)] = m;
            Language::UInt32 lp = lastPass.data()[static_cast<size_t>(r)];
            if (lp != 0) {
                Language::UInt32 prevTi = lp - static_cast<Language::UInt32>(1);
                (void)prevTi;
            }
            lastPass.data()[static_cast<size_t>(r)] = ti + static_cast<Language::UInt32>(1);
        }

        for (Language::USize r = 0; r < rc; ++r) {
            Language::UInt32 fm = firstMode.data()[static_cast<size_t>(r)];
            if (fm == static_cast<Language::UInt32>(0)) m_report.errors += static_cast<Language::USize>(1);
        }

        ::Foundation::Containers::Vector<Language::UInt32> prevMode(m_alloc);
        (void)prevMode.resize(rc);
        for (Language::USize i = 0; i < rc; ++i) prevMode.data()[static_cast<size_t>(i)] = static_cast<Language::UInt32>(~0u);
        for (Language::USize i = 0; i < up.size(); ++i) {
            Language::UInt32 r = ur.data()[static_cast<size_t>(i)];
            Language::UInt32 m = um.data()[static_cast<size_t>(i)];
            Language::UInt32 pm = prevMode.data()[static_cast<size_t>(r)];
            if (pm != static_cast<Language::UInt32>(~0u)) {
                if (pm == static_cast<Language::UInt32>(1) && m == static_cast<Language::UInt32>(1)) m_report.barrierCount += static_cast<Language::USize>(1);
                if (pm == static_cast<Language::UInt32>(1) && m == static_cast<Language::UInt32>(0)) m_report.barrierCount += static_cast<Language::USize>(1);
            }
            prevMode.data()[static_cast<size_t>(r)] = m;
        }

        m_report.passCount = static_cast<Language::USize>(n);
        return true;
    }

    bool FrameGraphValidator::CheckAsyncAlias(const FrameGraph& fg) noexcept {
        auto up = fg.UsagePass();
        auto ur = fg.UsageRes();
        auto order = fg.Order();
        Language::USize n = static_cast<Language::USize>(order.size());
        ::Foundation::Containers::Vector<Language::UInt32> topoIndex(m_alloc);
        (void)topoIndex.resize(n);
        for (Language::USize i = 0; i < n; ++i) topoIndex.data()[static_cast<size_t>(order.data()[static_cast<size_t>(i)])] = static_cast<Language::UInt32>(i);

        Language::USize rc = fg.ResourceCount();
        ::Foundation::Containers::Vector<Language::UInt32> start(m_alloc);
        ::Foundation::Containers::Vector<Language::UInt32> end(m_alloc);
        (void)start.resize(rc);
        (void)end.resize(rc);
        for (Language::USize i = 0; i < rc; ++i) { start.data()[static_cast<size_t>(i)] = static_cast<Language::UInt32>(n); end.data()[static_cast<size_t>(i)] = 0; }
        for (Language::USize i = 0; i < up.size(); ++i) {
            Language::UInt32 p = up.data()[static_cast<size_t>(i)];
            Language::UInt32 r = ur.data()[static_cast<size_t>(i)];
            Language::UInt32 t = topoIndex.data()[static_cast<size_t>(p)];
            Language::UInt32 s = start.data()[static_cast<size_t>(r)];
            Language::UInt32 e = end.data()[static_cast<size_t>(r)];
            if (t < s) start.data()[static_cast<size_t>(r)] = t;
            if (t > e) end.data()[static_cast<size_t>(r)] = t;
        }

        ::Foundation::Containers::Vector<Language::UInt32> orderRes(m_alloc);
        for (Language::USize i = 0; i < rc; ++i) (void)orderRes.push_back(static_cast<Language::UInt32>(i));
        for (Language::USize i = 0; i + 1 < orderRes.size(); ++i) {
            for (Language::USize j = i + 1; j < orderRes.size(); ++j) {
                if (start.data()[static_cast<size_t>(orderRes.data()[static_cast<size_t>(j)])] < start.data()[static_cast<size_t>(orderRes.data()[static_cast<size_t>(i)])]) {
                    Language::UInt32 tmp = orderRes.data()[static_cast<size_t>(i)];
                    orderRes.data()[static_cast<size_t>(i)] = orderRes.data()[static_cast<size_t>(j)];
                    orderRes.data()[static_cast<size_t>(j)] = tmp;
                }
            }
        }

        ::Foundation::Containers::Vector<Language::UInt32> slotEnd(m_alloc);
        ::Foundation::Containers::Vector<Language::UInt32> resSlot(m_alloc);
        (void)resSlot.resize(rc);
        for (Language::USize i = 0; i < rc; ++i) resSlot.data()[static_cast<size_t>(i)] = static_cast<Language::UInt32>(~0u);
        for (Language::USize idx = 0; idx < orderRes.size(); ++idx) {
            Language::UInt32 r = orderRes.data()[static_cast<size_t>(idx)];
            Language::UInt32 s = start.data()[static_cast<size_t>(r)];
            Language::UInt32 e = end.data()[static_cast<size_t>(r)];
            Language::UInt32 assigned = static_cast<Language::UInt32>(~0u);
            for (Language::USize k = 0; k < slotEnd.size(); ++k) {
                Language::UInt32 se = slotEnd.data()[static_cast<size_t>(k)];
                if (se < s) { assigned = static_cast<Language::UInt32>(k); break; }
            }
            if (assigned == static_cast<Language::UInt32>(~0u)) { assigned = static_cast<Language::UInt32>(slotEnd.size()); (void)slotEnd.push_back(e); }
            else { slotEnd.data()[static_cast<size_t>(assigned)] = e; }
            resSlot.data()[static_cast<size_t>(r)] = assigned;
        }

        m_report.aliasSafe = static_cast<Language::USize>(slotEnd.size());
        m_report.aliasConflict = static_cast<Language::USize>(0);
        return true;
    }

    Language::StatusResult<Report> FrameGraphValidator::GetReport() noexcept { return Language::StatusResult<Report>::Ok(m_report); }
}