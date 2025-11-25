module;
export module Engine.Renderer:Validator;

import Language;
import :FrameGraph;
import Foundation.Memory;

export namespace Engine::Renderer {
    struct Report {
        Language::USize passCount{0};
        Language::USize barrierCount{0};
        Language::USize aliasSafe{0};
        Language::USize aliasConflict{0};
        Language::USize errors{0};
    };

    class FrameGraphValidator {
    public:
        explicit FrameGraphValidator(::Foundation::Memory::IAllocator* alloc) noexcept;
        bool CheckConflicts(const FrameGraph& fg) noexcept;
        bool CheckAsyncAlias(const FrameGraph& fg) noexcept;
        Language::StatusResult<Report> GetReport() noexcept;
    private:
        ::Foundation::Memory::IAllocator* m_alloc{};
        Report m_report{};
    };
}