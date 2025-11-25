module;
export module Engine.Runtime:Runtime;

import Language;
import Foundation.Profiling;
import Foundation.Memory;
import :Types;

export namespace Engine {
    struct EngineRuntime {
        bool Initialize(const CoreConfig&) noexcept;
        bool RegisterSystem(ISystem*, Language::Int32 order) noexcept;
        void BeginFrame() noexcept;
        void Tick(float dt) noexcept;
        void EndFrame() noexcept;
        void Shutdown() noexcept;
        Language::USize SystemCount() const noexcept;
    private:
        ISystem** m_systems{nullptr};
        Language::Int32* m_orders{nullptr};
        Language::USize m_count{0};
        Language::USize m_capacity{0};
        CoreConfig m_cfg{};
        CoreFrameCtx m_ctx{};
        Foundation::Profiling::FrameProfiler m_prof{};
        Language::Scoped<::Foundation::Memory::IAllocator, ::Foundation::Memory::AllocatorDeleter> m_allocScoped;
        ::Foundation::Memory::IAllocator* m_alloc{nullptr};
    };
}