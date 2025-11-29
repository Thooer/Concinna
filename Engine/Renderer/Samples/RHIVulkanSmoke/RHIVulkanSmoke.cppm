module;
export module Engine.Renderer.RHIVulkanSmoke;

import Lang;
import <cstdio>;
import Foundation.Time;
import Foundation.IO;
import Foundation.Memory;
import Prm.Window;
import Engine.Renderer;

namespace Nova::Samples::RendererRHI {
  static void Log(const char* s) noexcept {
    (void)Foundation::IO::Path::CreateDirectory(Language::StringView{"Build"});
    (void)Foundation::IO::Path::CreateDirectory(Language::StringView{"Build/Reports"});
    auto fr = Foundation::IO::File::Open(Language::StringView{"Build/Reports/RendererSmoke.log"}, Foundation::IO::FileOpenMode::Append, Foundation::IO::FileShareMode::ReadWrite);
    if (fr.IsOk()) {
      auto fh = fr.OkValue();
      const Language::USize len = static_cast<Language::USize>(std::strlen(s));
      (void)Foundation::IO::File::Write(fh, Language::Span<const Language::Byte, Language::DynamicExtent>{ reinterpret_cast<const Language::Byte*>(s), len });
      (void)Foundation::IO::File::Write(fh, Language::Span<const Language::Byte, Language::DynamicExtent>{ reinterpret_cast<const Language::Byte*>("\n"), static_cast<Language::USize>(1) });
      (void)Foundation::IO::File::Close(fh);
    }
  }
  static Language::Int64 WndProc(void*, Language::UInt32, Language::UIntPtr, Language::IntPtr) { return 0; }
export bool Run() noexcept {
    Prm::WindowDesc wd{}; wd.width = 800; wd.height = 600; wd.visible = true; wd.resizable = false;
    auto whr = Prm::Window::Create(wd, &WndProc);
    void* hwnd = nullptr;
    bool headless = false;
    Prm::WindowHandle wh{};
    if (!whr.IsOk()) { Log("WINFAIL"); Log("HEADLESS"); headless = true; }
    else {
      wh = whr.OkValue();
      (void)Prm::Window::Show(wh);
      (void)Prm::Window::SetTitle(wh, Language::StringView{"RHI:WINOK"});
      auto nat = Prm::Window::Native(wh);
      if (!nat.IsOk()) { Log("NATFAIL"); (void)Prm::Window::SetTitle(wh, Language::StringView{"RHI:NATFAIL"}); Prm::Window::Destroy(wh); headless = true; }
      else { hwnd = nat.OkValue(); }
    }

    Engine::Renderer::RHI::DeviceCreateInfo dci{};
    auto dr = Engine::Renderer::RHI::CreateDevice(dci);
    if (!dr.IsOk()) { Log("DEVFAIL"); (void)Prm::Window::SetTitle(wh, Language::StringView{"RHI:DEVFAIL"}); Prm::Window::Destroy(wh); return false; }
    auto dev = dr.OkValue(); Log("DEVOK");
    if (!headless) (void)Prm::Window::SetTitle(wh, Language::StringView{"RHI:DEVOK"});

    Engine::Renderer::RHI::SurfaceInfo si{}; si.hwnd = hwnd; si.width = wd.width; si.height = wd.height;
    auto sr = Engine::Renderer::RHI::CreateSwapchain(dev, si); Log("SWAPCALL");
    if (!sr.IsOk()) { Log("SWAPFAIL"); if (!headless) (void)Prm::Window::SetTitle(wh, Language::StringView{"RHI:SWPFAIL"}); Engine::Renderer::RHI::DestroyDevice(dev); if (!headless) Prm::Window::Destroy(wh); return false; }
    auto sc = sr.OkValue(); Log("SWAPOK");
    if (!headless) (void)Prm::Window::SetTitle(wh, Language::StringView{"RHI:SWPOK"});

    auto qr = Engine::Renderer::RHI::GetQueue(dev, Engine::Renderer::RHI::QueueType::Graphics); Log("QCALL");
    if (!qr.IsOk()) { Log("QFAIL"); if (!headless) (void)Prm::Window::SetTitle(wh, Language::StringView{"RHI:QFAIL"}); Engine::Renderer::RHI::DestroySwapchain(dev, sc); Engine::Renderer::RHI::DestroyDevice(dev); if (!headless) Prm::Window::Destroy(wh); return false; }
    auto q = qr.OkValue(); Log("QOK");
    if (!headless) (void)Prm::Window::SetTitle(wh, Language::StringView{"RHI:QOK"});

    Engine::Renderer::RHI::CommandList cmd{};
    cmd.ClearColor(0.1f, 0.2f, 0.4f, 1.0f);

    // FrameGraph + Validator 集成：构建两 Pass 与一个资源，执行编译与验证
    auto ar = ::Foundation::Memory::CreateLinearAllocator(static_cast<Language::USize>(1ull << 20));
    ::Foundation::Memory::IAllocator* alloc = nullptr;
    if (ar.IsOk()) alloc = ar.OkValue();
    Engine::Renderer::Report vrep{}; bool haveRep = false;
    {
      Log("FGSTART"); Engine::Renderer::FrameGraph fg(alloc);
      Engine::Renderer::PassDependency deps[1]{}; deps[0].dependsOn = static_cast<Language::UInt32>(0);
      auto p0 = fg.AddPass(Language::StringView{"Collect"}, Language::Span<const Engine::Renderer::PassDependency, Language::DynamicExtent>{ deps, 0 });
      auto p1 = fg.AddPass(Language::StringView{"Draw"}, Language::Span<const Engine::Renderer::PassDependency, Language::DynamicExtent>{ deps, 1 });
      Engine::Renderer::ResourceDesc rd{}; rd.bytes = static_cast<Language::USize>(8 * 1024 * 1024); rd.type = static_cast<Language::UInt32>(0);
      auto r0 = fg.AddResource(rd);
      (void)fg.WriteResource(p0, r0);
      (void)fg.ReadResource(p1, r0);
      (void)fg.Compile(); Log("FGCOMPILE");
      Engine::Renderer::FrameGraphValidator val{alloc};
      (void)val.CheckConflicts(fg); Log("FGCHK1");
      (void)val.CheckAsyncAlias(fg); Log("FGCHK2");
      auto rep = val.GetReport(); Log("FGDONE");
      if (rep.IsOk()) { vrep = rep.OkValue(); haveRep = true; }
    }

    Language::USize frames = static_cast<Language::USize>(300);
    double build_ms = 0.0;
    double submit_ms = 0.0;
    Language::USize passes = 0;
  Log("LOOP");
  for (Language::USize i = 0; i < frames; ++i) {
      cmd.Begin();
      auto t0 = Foundation::Time::SteadyClock::Now();
      cmd.DrawTriangle2D(100.0f, 100.0f, 700.0f, 100.0f, 400.0f, 500.0f, 1.0f, 0.6f, 0.2f, 1.0f);
      cmd.End();
      auto t1 = Foundation::Time::SteadyClock::Now();
      Log("SUBMIT"); (void)Engine::Renderer::RHI::Submit(q, cmd, nullptr, sc); Log("SUBMITOK");
      auto t2 = Foundation::Time::SteadyClock::Now();
      build_ms += Foundation::Time::SteadyClock::ToMilliseconds(Foundation::Time::SteadyClock::Delta(t0, t1));
      submit_ms += Foundation::Time::SteadyClock::ToMilliseconds(Foundation::Time::SteadyClock::Delta(t1, t2));
      passes += static_cast<Language::USize>(1);
      if (!headless) (void)Prm::Window::ProcessOneMessage(wh);
    }

    (void)Foundation::IO::Path::CreateDirectory(Language::StringView{"Build"});
    (void)Foundation::IO::Path::CreateDirectory(Language::StringView{"Build/Reports"});
    auto fr = Foundation::IO::File::Open(Language::StringView{"Build/Reports/Mainline-C-Graphics.json"}, Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::ReadWrite);
    if (fr.IsOk()) {
      auto fh = fr.OkValue();
      auto writeStr = [&](const char* s) noexcept {
        const Language::USize len = static_cast<Language::USize>(std::strlen(s));
        (void)Foundation::IO::File::Write(fh, Language::Span<const Language::Byte, Language::DynamicExtent>{ reinterpret_cast<const Language::Byte*>(s), len });
      };
      char num[64]{};
      writeStr("{");
      writeStr("\"passes_count\":"); int n1 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)passes); writeStr(num);
      writeStr(",\"build_ms\":"); int n2 = std::snprintf(num, sizeof(num), "%.2f", build_ms); writeStr(num);
      writeStr(",\"submit_ms\":"); int n3 = std::snprintf(num, sizeof(num), "%.2f", submit_ms); writeStr(num);
      writeStr(",\"validator\":{");
      if (haveRep) {
        writeStr("\"passCount\":"); int m1 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)vrep.passCount); writeStr(num);
        writeStr(",\"barrierCount\":"); int m2 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)vrep.barrierCount); writeStr(num);
        writeStr(",\"aliasSafe\":"); int m3 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)vrep.aliasSafe); writeStr(num);
        writeStr(",\"aliasConflict\":"); int m4 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)vrep.aliasConflict); writeStr(num);
        writeStr(",\"errors\":"); int m5 = std::snprintf(num, sizeof(num), "%llu", (unsigned long long)vrep.errors); writeStr(num);
      } else {
        writeStr("\"error\":\"validator_failed\"");
      }
      writeStr("}" );
      writeStr("}");
      (void)Foundation::IO::File::Close(fh);
    }

    Engine::Renderer::RHI::DestroySwapchain(dev, sc);
    Engine::Renderer::RHI::DestroyDevice(dev);
    if (alloc) { ::Foundation::Memory::DestroyRuntimeAllocator(alloc); }
    if (!headless) Prm::Window::Destroy(wh);
    return true;
}
}
