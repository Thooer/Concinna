export module Engine.Runtime.LoopSmoke;

import Language;
import Foundation.Time;
import Foundation.Containers;
import Engine.Runtime;

namespace Nova::Samples::Engine {
    struct DummySystem : ::Engine::ISystem {
        bool Initialize(const ::Engine::CoreConfig&) noexcept { return true; }
        void Tick(float) noexcept {}
        void Shutdown() noexcept {}
    };

    export bool Run() {
        ::Engine::EngineRuntime eng{};
        ::Engine::CoreConfig cfg{};
        if (!eng.Initialize(cfg)) return false;
        DummySystem s1{};
        DummySystem s2{};
        if (!eng.RegisterSystem(&s1, 10)) return false;
        if (!eng.RegisterSystem(&s2, 20)) return false;
        for (int i=0;i<3;++i) {
            eng.BeginFrame();
            eng.Tick(0.016f);
            eng.EndFrame();
        }
        eng.Shutdown();
        return eng.SystemCount()==0 || eng.SystemCount()==2;
    }
}