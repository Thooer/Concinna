module;
export module Engine.Runtime:Runtime;

import Lang;
import Prm.Time;
import :Types;

export namespace Engine {
    struct EngineRuntime {
        bool Initialize(const CoreConfig&) noexcept;
        bool RegisterSystem(ISystem*, Int32 order) noexcept;
        void BeginFrame() noexcept;
        void Tick(float dt) noexcept;
        void EndFrame() noexcept;
        void Shutdown() noexcept;
        USize SystemCount() const noexcept;
    private:
        ISystem** m_systems{nullptr};
        Int32* m_orders{nullptr};
        USize m_count{0};
        USize m_capacity{0};
        CoreConfig m_cfg{};
        CoreFrameCtx m_ctx{};
    };
}
