export module Engine.Runtime.ChainSmoke;

import Language;
import Engine.Runtime;

namespace Nova::Samples::EngineChain {
    struct SysA : ::Engine::ISystem { bool Initialize(const ::Engine::CoreConfig&) noexcept { return true; } void Tick(float) noexcept {} void Shutdown() noexcept {} };
    struct SysB : ::Engine::ISystem { bool Initialize(const ::Engine::CoreConfig&) noexcept { return true; } void Tick(float) noexcept {} void Shutdown() noexcept {} };
    struct SysC : ::Engine::ISystem { bool Initialize(const ::Engine::CoreConfig&) noexcept { return true; } void Tick(float) noexcept {} void Shutdown() noexcept {} };

    export bool Run() {
        ::Engine::EngineRuntime eng{}; ::Engine::CoreConfig cfg{}; if (!eng.Initialize(cfg)) return false;
        SysA a{}; SysB b{}; SysC c{};
        if (!eng.RegisterSystem(&a, 30)) return false;
        if (!eng.RegisterSystem(&b, 10)) return false;
        if (!eng.RegisterSystem(&c, 20)) return false;
        eng.BeginFrame(); eng.Tick(0.016f); eng.EndFrame(); eng.Shutdown();
        return true;
    }
}