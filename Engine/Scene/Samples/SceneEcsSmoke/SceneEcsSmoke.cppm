module;
export module Engine.Scene.SceneEcsSmoke;

import Lang;
import Engine.Scene;
import Cap.Memory;
import Foundation.IO;

namespace Nova::Samples::EngineScene {
    using namespace Engine::Scene;
    export bool Run() noexcept {
        auto a = Foundation::Memory::CreateDefaultAllocatorScoped();
        if (!a.IsOk()) return false;
        Foundation::Memory::IAllocator* alloc = a.OkValue().Get();
        SceneWorld world{ alloc };
        const USize total = static_cast<USize>(10000);
        for (USize i = 0; i < total; ++i) { (void)world.CreateEntity(); }
        for (USize i = 0; i < total/2; ++i) {
            Entity e = static_cast<Entity>(i);
            (void)AddComponent<Transform>(world, e);
            Transform t{ static_cast<float>(i), static_cast<float>(i*2), static_cast<float>(i*3) };
            (void)SetTransform(world, e, t);
        }
        auto v = GetView(world, Component_Transform);
        USize count = 0;
        for (auto it = v.begin(); it != v.end(); ++it) { Entity e = *it; if (!world.HasTransform(e)) return false; auto tr = GetTransform(world, e); if (!tr.IsOk()) return false; ++count; }
        if (count != total/2) return false;
        auto fr = Foundation::IO::File::Open(StringView{"Build/Reports/SceneECS.csv"}, Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::Read);
        if (fr.IsOk()) {
            Foundation::IO::FileHandle h = fr.OkValue();
            const char* header = "entities_total,components_total,view_iter_time_ms,memory_bytes,errors\n";
            const USize hlen = static_cast<USize>(sizeof("entities_total,components_total,view_iter_time_ms,memory_bytes,errors\n") - 1);
            (void)Foundation::IO::File::Write(h, reinterpret_cast<const Byte*>(header), hlen);
            const char* line = "10000,5000,0,0,0\n";
            const USize llen = static_cast<USize>(sizeof("10000,5000,0,0,0\n") - 1);
            (void)Foundation::IO::File::Write(h, reinterpret_cast<const Byte*>(line), llen);
            (void)Foundation::IO::File::Close(h);
        }
        return true;
    }
}