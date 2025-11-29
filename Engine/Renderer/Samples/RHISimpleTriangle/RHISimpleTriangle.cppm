module;
export module Eng.Renderer.RHISimpleTriangle;

import Lang;
import Lang.Text;
import Prm.Window;
import Prm.Time;
import Eng.Renderer;

static volatile bool g_quit = false;
static Int64 WndProc(void* h, UInt32 m, UIntPtr, IntPtr) { if (m==0x0010u || m==0x0002u) { g_quit=true; } return 0; }

export bool Run() noexcept {
    Prm::WindowDesc wd{}; wd.width=800; wd.height=600; wd.visible=true; wd.resizable=true;
    auto wr = Prm::Window::Create(wd, &WndProc); if (!wr.IsOk()) return false; auto wh = wr.Value();
    (void)Prm::Window::Show(wh);
    (void)Prm::Window::SetTitle(wh, StringView("RHI:WINOK"));
    Eng::Renderer::RHI::DeviceCreateInfo dci{};
    auto dr = Eng::Renderer::RHI::CreateDevice(dci); if (!dr.IsOk()) { (void)Prm::Window::SetTitle(wh, StringView("RHI:DEVFAIL")); Prm::Window::Destroy(wh); return false; }
    auto dev = dr.Value();
    auto n = Prm::Window::Native(wh); if (!n.IsOk()) { (void)Prm::Window::SetTitle(wh, StringView("RHI:NATFAIL")); Eng::Renderer::RHI::DestroyDevice(dev); Prm::Window::Destroy(wh); return false; }
    Eng::Renderer::RHI::SurfaceInfo si{}; si.hwnd = n.Value(); si.width = wd.width; si.height = wd.height;
    auto sr = Eng::Renderer::RHI::CreateSwapchain(dev, si); if (!sr.IsOk()) { (void)Prm::Window::SetTitle(wh, StringView("RHI:SWAPFAIL")); Eng::Renderer::RHI::DestroyDevice(dev); Prm::Window::Destroy(wh); return false; }
    auto sc = sr.Value();
    auto qr = Eng::Renderer::RHI::GetQueue(dev, Eng::Renderer::RHI::QueueType::Graphics); if (!qr.IsOk()) { (void)Prm::Window::SetTitle(wh, StringView("RHI:QUEUEFAIL")); Eng::Renderer::RHI::DestroySwapchain(dev, sc); Eng::Renderer::RHI::DestroyDevice(dev); Prm::Window::Destroy(wh); return false; }
    auto q = qr.Value();
    Eng::Renderer::RHI::CommandList cmd{};
    float x0 = wd.width*0.5f; float y0 = wd.height*0.2f; float x1 = wd.width*0.2f; float y1 = wd.height*0.8f; float x2 = wd.width*0.8f; float y2 = wd.height*0.8f;
    (void)Prm::Window::SetTitle(wh, StringView("RHI:RUN"));
    UInt32 tick = 0u;
    while(!g_quit) {
        UInt32 handled = 0u; while (Prm::Window::ProcessOneMessage(wh) && handled < 64u) { ++handled; }
        cmd.Begin();
        cmd.ClearColor(0.05f,0.05f,0.08f,1.0f);
        float dy = static_cast<float>((tick % 120u)) * 0.5f;
        cmd.DrawTriangle2D(x0,y0+dy,x1,y1+dy,x2,y2+dy,1.0f,0.25f,0.1f,1.0f);
        cmd.End();
        auto st = Eng::Renderer::RHI::Submit(q, cmd, nullptr, sc);
        if (!st.Ok()) { (void)Prm::Window::SetTitle(wh, StringView("RHI:SUBMITFAIL")); }
        ++tick;
        Prm::SleepMs(16);
    }
    Eng::Renderer::RHI::DestroySwapchain(dev, sc);
    Eng::Renderer::RHI::DestroyDevice(dev);
    Prm::Window::Destroy(wh);
    return true;
}
