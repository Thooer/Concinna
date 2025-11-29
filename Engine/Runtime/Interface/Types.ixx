module;
export module Engine.Runtime:Types;

import Lang;
import Prm.Time;

export namespace Engine {
    using USize = ::USize;
    using Int32 = ::Int32;

    struct CoreConfig { USize maxSystems{64}; };
    struct CoreFrameCtx { Int64 frameStart{0}; Int64 frameDuration{0}; };
    struct FrameContext { Int64 ts{0}; void* bus{nullptr}; };

    struct ISystem {
        virtual ~ISystem() = default;
        virtual bool Initialize(const CoreConfig&) noexcept = 0;
        virtual void Tick(float dt) noexcept = 0;
        virtual void Shutdown() noexcept = 0;
    };
}
