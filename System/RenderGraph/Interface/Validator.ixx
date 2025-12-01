module;
export module Sys.RenderGraph:Validator;

import Lang;
import Cap.Memory;
import :FrameGraph;

export namespace Sys {
    struct Report { USize passCount{}, barrierCount{}, aliasSafe{}, aliasConflict{}, errors{}; };
    class FrameGraphValidator {
    public:
        explicit FrameGraphValidator(Cap::Allocator alloc) noexcept;
        bool CheckConflicts(const FrameGraph& fg) noexcept;
        bool CheckAsyncAlias(const FrameGraph& fg) noexcept;
        Cap::StatusResult<Report> GetReport() noexcept;
    private:
        Cap::Allocator m_alloc{};
        Report m_report{};
    };
}
