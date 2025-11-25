module;
export module Engine.Runtime:Types;

import Language;
import Foundation.Time;
import Foundation.Memory;

export namespace Engine {
    using Language::USize;
    using Language::Int32;

    struct CoreConfig { USize maxSystems{64}; };
    struct CoreFrameCtx { Foundation::Time::Timestamp frameStart{0}; Foundation::Time::Duration frameDuration{0}; };
    struct FrameContext { Foundation::Time::Timestamp ts{0}; ::Foundation::Memory::IAllocator* alloc{nullptr}; void* bus{nullptr}; };

    struct ISystem {
        virtual ~ISystem() = default;
        virtual bool Initialize(const CoreConfig&) noexcept = 0;
        virtual void Tick(float dt) noexcept = 0;
        virtual void Shutdown() noexcept = 0;
    };
}