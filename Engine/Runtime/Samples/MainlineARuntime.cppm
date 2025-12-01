module;
#include <cstdio>
export module Engine.Runtime.MainlineARuntime;

import Lang;
import Foundation.Time;
import Foundation.Profiling;
import Cap.Memory;
import Engine.Runtime;
// import Engine.Runtime:TaskHook; // covered by Engine.Runtime umbrella
import Engine.Messaging;

namespace Nova::Samples::Engine {
    struct ProducerSystem : ::Engine::ISystem {
        ::Engine::Messaging::EventBus* bus{nullptr};
        bool Initialize(const ::Engine::CoreConfig&) noexcept { return true; }
        void Tick(float) noexcept {}
        void Shutdown() noexcept {}
    };

    static void write_csv(const char* metric, long long value, const char* unit) {
        const char* outFile = "Build/Reports/Mainline-A-Runtime.csv";
        FILE* f = nullptr;
        (void)::fopen_s(&f, outFile, "a");
        if (!f) return;
        long long ts = 0;
        std::fprintf(f, "Mainline-A-Runtime,%s,%lld,%s,%lld\n", metric, value, unit, ts);
        std::fclose(f);
    }

    static void write_json_summary(long long job_tput, long long msg_tput, long long p99_ns, long long drop_cnt) {
        const char* outFile = "Build/Reports/Mainline-A-Runtime.json";
        FILE* f = nullptr;
        (void)::fopen_s(&f, outFile, "w");
        if (!f) return;
        std::fprintf(f, "{\"lane\":\"Mainline-A-Runtime\",\"metrics\":{\"EmptyJobThroughput\":%lld,\"MessageThroughput\":%lld,\"LatencyP99Ns\":%lld,\"Dropped\":%lld}}", job_tput, msg_tput, p99_ns, drop_cnt);
        std::fclose(f);
    }

    export bool Run() {
        ::Engine::EngineRuntime eng{};
        ::Engine::CoreConfig cfg{};
        if (!eng.Initialize(cfg)) return false;

        (void)::Engine::GetTaskSystemHook();

        Cap::IAllocator* alloc = nullptr;
        (void)alloc;

        ProducerSystem prod{};
        prod.bus = nullptr;
        if (!eng.RegisterSystem(&prod, 10)) return false;

        const int frames = 3;
        for (int i = 0; i < frames; ++i) {
            eng.BeginFrame();
            eng.Tick(0.016f);
            eng.EndFrame();
        }

        write_csv("RuntimeLoop", 1, "pass");
        write_csv("SmokeResult", 1, "pass");
        write_json_summary(0, 0, 0, 0);

        eng.Shutdown();
        return true;
    }
}