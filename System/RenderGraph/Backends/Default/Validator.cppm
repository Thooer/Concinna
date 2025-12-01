module;
module Sys.RenderGraph;

import Lang;
import Cap.Memory;
import :Validator;
import :FrameGraph;
import Containers;
// Vector is re-exported by primary Containers module

namespace Sys {
    FrameGraphValidator::FrameGraphValidator(Cap::Allocator alloc) noexcept : m_alloc(alloc), m_report{} {}
    bool FrameGraphValidator::CheckConflicts(const FrameGraph& fg) noexcept {
        auto up = fg.UsagePass();
        auto ur = fg.UsageRes();
        auto um = fg.UsageMode();
        auto order = fg.Order();
        USize n = static_cast<USize>(order.size());
        ::Containers::Vector<UInt32> topoIndex(m_alloc);
        (void)topoIndex.Resize(n);
        for (USize i = 0; i < n; ++i) topoIndex.Data()[static_cast<size_t>(order.data()[static_cast<size_t>(i)])] = static_cast<UInt32>(i);

        USize rc = fg.ResourceCount();
        ::Containers::Vector<UInt32> firstMode(m_alloc);
        ::Containers::Vector<UInt32> lastPass(m_alloc);
        (void)firstMode.Resize(rc);
        (void)lastPass.Resize(rc);
        for (USize i = 0; i < rc; ++i) { firstMode.Data()[static_cast<size_t>(i)] = static_cast<UInt32>(~0u); lastPass.Data()[static_cast<size_t>(i)] = 0; }

        for (USize i = 0; i < up.size(); ++i) {
            UInt32 r = ur.data()[static_cast<size_t>(i)];
            UInt32 p = up.data()[static_cast<size_t>(i)];
            UInt32 m = um.data()[static_cast<size_t>(i)];
            UInt32 ti = topoIndex.Data()[static_cast<size_t>(p)];
            if (firstMode.Data()[static_cast<size_t>(r)] == static_cast<UInt32>(~0u)) firstMode.Data()[static_cast<size_t>(r)] = m;
            UInt32 lp = lastPass.Data()[static_cast<size_t>(r)];
            if (lp != 0) {
                UInt32 prevTi = lp - static_cast<UInt32>(1);
                (void)prevTi;
            }
            lastPass.Data()[static_cast<size_t>(r)] = ti + static_cast<UInt32>(1);
        }

        for (USize r = 0; r < rc; ++r) {
            UInt32 fm = firstMode.Data()[static_cast<size_t>(r)];
            if (fm == static_cast<UInt32>(0)) m_report.errors += static_cast<USize>(1);
        }

        ::Containers::Vector<UInt32> prevMode(m_alloc);
        (void)prevMode.Resize(rc);
        for (USize i = 0; i < rc; ++i) prevMode.Data()[static_cast<size_t>(i)] = static_cast<UInt32>(~0u);
        for (USize i = 0; i < up.size(); ++i) {
            UInt32 r = ur.data()[static_cast<size_t>(i)];
            UInt32 m = um.data()[static_cast<size_t>(i)];
            UInt32 pm = prevMode.Data()[static_cast<size_t>(r)];
            if (pm != static_cast<UInt32>(~0u)) {
                if (pm == static_cast<UInt32>(1) && m == static_cast<UInt32>(1)) m_report.barrierCount += static_cast<USize>(1);
                if (pm == static_cast<UInt32>(1) && m == static_cast<UInt32>(0)) m_report.barrierCount += static_cast<USize>(1);
            }
            prevMode.Data()[static_cast<size_t>(r)] = m;
        }

        m_report.passCount = static_cast<USize>(n);
        return true;
    }

    bool FrameGraphValidator::CheckAsyncAlias(const FrameGraph& fg) noexcept {
        auto up = fg.UsagePass();
        auto ur = fg.UsageRes();
        auto order = fg.Order();
        USize n = static_cast<USize>(order.size());
        ::Containers::Vector<UInt32> topoIndex(m_alloc);
        (void)topoIndex.Resize(n);
        for (USize i = 0; i < n; ++i) topoIndex.Data()[static_cast<size_t>(order.data()[static_cast<size_t>(i)])] = static_cast<UInt32>(i);

        USize rc = fg.ResourceCount();
        ::Containers::Vector<UInt32> start(m_alloc);
        ::Containers::Vector<UInt32> end(m_alloc);
        (void)start.Resize(rc);
        (void)end.Resize(rc);
        for (USize i = 0; i < rc; ++i) { start.Data()[static_cast<size_t>(i)] = static_cast<UInt32>(n); end.Data()[static_cast<size_t>(i)] = 0; }
        for (USize i = 0; i < up.size(); ++i) {
            UInt32 p = up.data()[static_cast<size_t>(i)];
            UInt32 r = ur.data()[static_cast<size_t>(i)];
            UInt32 t = topoIndex.Data()[static_cast<size_t>(p)];
            UInt32 s = start.Data()[static_cast<size_t>(r)];
            UInt32 e = end.Data()[static_cast<size_t>(r)];
            if (t < s) start.Data()[static_cast<size_t>(r)] = t;
            if (t > e) end.Data()[static_cast<size_t>(r)] = t;
        }

        ::Containers::Vector<UInt32> orderRes(m_alloc);
        (void)orderRes.Reserve(rc);
        for (USize i = 0; i < rc; ++i) orderRes.EmplaceBackUnsafe(static_cast<UInt32>(i));
        for (USize i = 0; i + 1 < orderRes.Size(); ++i) {
            for (USize j = i + 1; j < orderRes.Size(); ++j) {
                if (start.Data()[static_cast<size_t>(orderRes.Data()[static_cast<size_t>(j)])] < start.Data()[static_cast<size_t>(orderRes.Data()[static_cast<size_t>(i)])]) {
                    UInt32 tmp = orderRes.Data()[static_cast<size_t>(i)];
                    orderRes.Data()[static_cast<size_t>(i)] = orderRes.Data()[static_cast<size_t>(j)];
                    orderRes.Data()[static_cast<size_t>(j)] = tmp;
                }
            }
        }

        ::Containers::Vector<UInt32> slotEnd(m_alloc);
        ::Containers::Vector<UInt32> resSlot(m_alloc);
        (void)resSlot.Resize(rc);
        for (USize i = 0; i < rc; ++i) resSlot.Data()[static_cast<size_t>(i)] = static_cast<UInt32>(~0u);
        for (USize idx = 0; idx < orderRes.Size(); ++idx) {
            UInt32 r = orderRes.Data()[static_cast<size_t>(idx)];
            UInt32 s = start.Data()[static_cast<size_t>(r)];
            UInt32 e = end.Data()[static_cast<size_t>(r)];
            UInt32 assigned = static_cast<UInt32>(~0u);
            for (USize k = 0; k < slotEnd.Size(); ++k) {
                UInt32 se = slotEnd.Data()[static_cast<size_t>(k)];
                if (se < s) { assigned = static_cast<UInt32>(k); break; }
            }
            if (assigned == static_cast<UInt32>(~0u)) { assigned = static_cast<UInt32>(slotEnd.Size()); slotEnd.EmplaceBackUnsafe(e); }
            else { slotEnd.Data()[static_cast<size_t>(assigned)] = e; }
            resSlot.Data()[static_cast<size_t>(r)] = assigned;
        }

        m_report.aliasSafe = static_cast<USize>(slotEnd.Size());
        m_report.aliasConflict = static_cast<USize>(0);
        return true;
    }

    Cap::StatusResult<Report> FrameGraphValidator::GetReport() noexcept { return Cap::StatusResult<Report>::Ok(m_report); }
}
