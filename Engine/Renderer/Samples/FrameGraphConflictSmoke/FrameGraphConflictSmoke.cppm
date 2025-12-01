module;
export module Engine.Renderer.FrameGraphConflictSmoke;

import Lang;
import <cstdio>;
import Cap.Memory;
import Foundation.IO;
import Engine.Scene;
import Engine.Renderer;

namespace Nova::Samples::FrameGraphConflict {
  export bool Run() noexcept {
    auto ar = Cap::CreateLinearAllocator(static_cast<USize>(1ull << 20));
    if (!ar.IsOk()) return false;
    Cap::IAllocator* alloc = ar.OkValue();

    ::Engine::Scene::SceneWorld world(alloc);
    auto e1 = world.CreateEntity();
    (void)world.SetTransform(e1, ::Engine::Scene::Transform{0.0f, 0.0f, 0.0f});
    (void)world.BindMesh(e1, static_cast<UInt32>(1));
    (void)world.BindMaterial(e1, static_cast<UInt32>(10));
    auto view = world.GetView();

    ::Engine::Renderer::FrameGraph fg(alloc);
    ::Engine::Renderer::PassId pA = fg.AddPass(StringView{"Draw"}, Span<const ::Engine::Renderer::PassDependency, DynamicExtent>{});
    ::Engine::Renderer::PassDependency dA{ pA };
    ::Engine::Renderer::PassId pB = fg.AddPass(StringView{"Draw"}, Span<const ::Engine::Renderer::PassDependency, DynamicExtent>(&dA, 1));
    ::Engine::Renderer::ResourceDesc rdesc{ static_cast<USize>(4ull << 20), static_cast<UInt32>(0) };
    ::Engine::Renderer::ResourceId rX = fg.AddResource(rdesc);
    (void)fg.ReadResource(pA, rX);
    (void)fg.WriteResource(pB, rX);
    if (!fg.Compile()) { delete alloc; return false; }
    (void)fg.Execute(view);

    ::Engine::Renderer::FrameGraphValidator v{alloc};
    (void)v.CheckConflicts(fg);
    (void)v.CheckAsyncAlias(fg);
    auto rr = v.GetReport();
    if (!rr.IsOk()) { delete alloc; return false; }
    auto rep = rr.OkValue();

    Byte buf[256]{};
    USize p = 0;
    auto put = [&](Char8 c) noexcept { buf[p++] = static_cast<Byte>(c); };
    auto puts = [&](const char* s) noexcept { while (*s) { put(static_cast<Char8>(*s++)); } };
    char num[64]{};
    puts("{");
    puts("\"passes_count\":"); int n1 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)rep.passCount); for (int i=0;i<n1;++i) put((Char8)num[i]);
    puts(",\"barrier_count\":"); int n2 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)rep.barrierCount); for (int i=0;i<n2;++i) put((Char8)num[i]);
    puts(",\"alias_safe\":"); int n3 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)rep.aliasSafe); for (int i=0;i<n3;++i) put((Char8)num[i]);
    puts(",\"alias_conflict\":"); int n4 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)rep.aliasConflict); for (int i=0;i<n4;++i) put((Char8)num[i]);
    puts(",\"errors\":["); int n5 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)rep.errors); for (int i=0;i<n5;++i) put((Char8)num[i]); puts("]}");

    (void)Foundation::IO::Path::CreateDirectory(StringView{"Build"});
    (void)Foundation::IO::Path::CreateDirectory(StringView{"Build/Reports"});
    auto fr = Foundation::IO::File::Open(StringView{"Build/Reports/Renderer.FrameGraph.json"}, Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::ReadWrite);
    if (fr.IsOk()) {
      auto fh = fr.OkValue();
      (void)Foundation::IO::File::Write(fh, Span<const Byte, DynamicExtent>{ buf, p });
      (void)Foundation::IO::File::Close(fh);
    }

    bool ok = (rep.errors > static_cast<USize>(0));
    delete alloc;
    return ok;
  }
}