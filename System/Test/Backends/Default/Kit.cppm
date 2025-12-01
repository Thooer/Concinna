module;
module Sys.Test;

import Lang;
import Prm.Window;
import Prm.Time;
import Sys.RHI;
import :Kit;

namespace Sys::Test {
    bool RunRHIWindowed(UInt32 width, UInt32 height, FrameFunc f) noexcept {
        Prm::WindowDesc wd{}; wd.width=width; wd.height=height; wd.visible=true; wd.resizable=true;
        volatile bool quit=false;
        auto WndProc = [](void* h, UInt32 m, UIntPtr, IntPtr) -> Int64 { (void)h; if (m==0x0010u || m==0x0002u) { return 1; } return 0; };
        auto wr = Prm::Window::Create(wd, WndProc); if (!wr.IsOk()) return false; auto wh = wr.Value(); (void)Prm::Window::Show(wh);
        auto dr = Sys::CreateDevice(); if (!dr.IsOk()) { Prm::Window::Destroy(wh); return false; } auto dev = dr.Value();
        auto n = Prm::Window::Native(wh); if (!n.IsOk()) { Sys::DestroyDevice(dev); Prm::Window::Destroy(wh); return false; }
        Sys::SurfaceInfo si{}; si.hwnd = n.Value(); si.width = width; si.height = height;
        auto sr = Sys::CreateSwapchain(dev, si); if (!sr.IsOk()) { Sys::DestroyDevice(dev); Prm::Window::Destroy(wh); return false; } auto sc = sr.Value();
        auto qr = Sys::GetQueue(dev, Sys::QueueType::Graphics); if (!qr.IsOk()) { Sys::DestroySwapchain(dev, sc); Sys::DestroyDevice(dev); Prm::Window::Destroy(wh); return false; } auto q = qr.Value();
        Sys::CommandList cmd{};
        UInt32 tick = 0u;
        while(!quit) {
            UInt32 handled = 0u; while (Prm::Window::ProcessOneMessage(wh) && handled < 64u) { ++handled; }
            if (WndProc(nullptr, 0x0000u, UIntPtr{}, IntPtr{})==1) { quit=true; }
            cmd.Begin();
            auto st = f(dev, q, sc, cmd, tick);
            cmd.End();
            (void)Sys::Submit(q, cmd, nullptr, sc);
            ++tick;
            Prm::SleepMs(16);
        }
        Sys::DestroySwapchain(dev, sc);
        Sys::DestroyDevice(dev);
        Prm::Window::Destroy(wh);
        return true;
    }
}
