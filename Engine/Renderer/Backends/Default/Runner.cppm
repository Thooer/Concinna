module;
module Eng.Renderer;

import Lang;
import Prm.Window;
import Prm.Time;
import Sys.RHI;
import :Runner;
import :Types;
namespace Eng {
    bool RunWindowed(UInt32 width, UInt32 height, FrameFunc f) noexcept {
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
        UInt32 tick = static_cast<UInt32>(0);
        while(!quit) {
            UInt32 handled = static_cast<UInt32>(0); while (Prm::Window::ProcessOneMessage(wh) && handled < static_cast<UInt32>(64)) { ++handled; }
            if (WndProc(nullptr, static_cast<UInt32>(0), UIntPtr{}, IntPtr{})==1) { quit=true; }
            cmd.Begin();
            Eng::SimpleFrame fr{};
            fr.screenW = static_cast<float>(width);
            fr.screenH = static_cast<float>(height);
            (void)f(fr, tick);
            cmd.ClearColor(fr.clear[0], fr.clear[1], fr.clear[2], fr.clear[3]);
            for (UInt32 i = 0; i < fr.triCount && i < Eng::kMaxTris; ++i) { const auto& t = fr.tris[i]; cmd.DrawTriangle(t.x0,t.y0,t.z0,t.x1,t.y1,t.z1,t.x2,t.y2,t.z2,t.r,t.g,t.b,t.a); }
            for (UInt32 i = 0; i < fr.rectCount && i < Eng::kMaxRects; ++i) { const auto& rc = fr.rects[i]; float x=rc.x,y=rc.y,w=rc.w,h=rc.h; float r=rc.r,g=rc.g,b=rc.b,a=rc.a; cmd.DrawTriangle(x,y,0.5f,x+w,y,0.5f,x,y+h,0.5f,r,g,b,a); cmd.DrawTriangle(x+w,y,0.5f,x+w,y+h,0.5f,x,y+h,0.5f,r,g,b,a); }
            for (UInt32 i = 0; i < fr.lineCount && i < Eng::kMaxLines; ++i) { const auto& ln = fr.lines[i]; float dx=ln.x1-ln.x0; float dy=ln.y1-ln.y0; float adx = dx>=0.0f?dx:-dx; float ady = dy>=0.0f?dy:-dy; float len = adx+ady; float nx = len>0.0f ? (-dy/len) : 0.0f; float ny = len>0.0f ? (dx/len) : 0.0f; float ox = nx*(ln.w*0.5f); float oy = ny*(ln.w*0.5f); float x0a = ln.x0-ox, y0a = ln.y0-oy; float x0b = ln.x0+ox, y0b = ln.y0+oy; float x1a = ln.x1-ox, y1a = ln.y1-oy; float x1b = ln.x1+ox, y1b = ln.y1+oy; cmd.DrawTriangle(x0a,y0a,0.5f,x1a,y1a,0.5f,x0b,y0b,0.5f,ln.r,ln.g,ln.b,ln.a); cmd.DrawTriangle(x1a,y1a,0.5f,x1b,y1b,0.5f,x0b,y0b,0.5f,ln.r,ln.g,ln.b,ln.a); }
            for (UInt32 i = 0; i < fr.glyphCount && i < Eng::kMaxGlyphs; ++i) { const auto& gq = fr.glyphs[i]; float x=gq.x,y=gq.y,w=gq.w,h=gq.h; float r=gq.r,g=gq.g,b=gq.b,a=gq.a; cmd.DrawTriangle(x,y,0.5f,x+w,y,0.5f,x,y+h,0.5f,r,g,b,a); cmd.DrawTriangle(x+w,y,0.5f,x+w,y+h,0.5f,x,y+h,0.5f,r,g,b,a); }
            cmd.End();
            (void)Sys::Submit(q, cmd, nullptr, sc);
            ++tick;
            Prm::SleepMs(static_cast<UInt32>(16));
        }
        Sys::DestroySwapchain(dev, sc);
        Sys::DestroyDevice(dev);
        Prm::Window::Destroy(wh);
        return true;
    }
}
