import Lang;
import Prm.Time;
import Engine.Runtime;

int main() {
    Engine::EngineRuntime runtime{};
    Engine::CoreConfig cfg{};
    if (!runtime.Initialize(cfg)) { return 1; }

    for (int i = 0; i < 120; ++i) {
        runtime.BeginFrame();
        runtime.Tick(1.0f / 60.0f);
        runtime.EndFrame();
        Prm::SleepMs(16);
    }

    runtime.Shutdown();
    return 0;
}

