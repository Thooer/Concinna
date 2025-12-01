module;
export module Pjt.Cubes;

import Lang;
import Eng.Renderer;
import Sys.Job;
import Prm.Time;
import Prm.System;
import Prm.IO;
 
 

namespace Pjt {
    struct CubeCmd { float mvp[16]; };
    static CubeCmd* gCmds = nullptr;
    static UInt32 gCount = 0;
    static float gView[16];
    static float gProj[16];
    struct GridState { float sx; float sz; UInt32 cols; float scale; };
    static GridState gs{ 2.5f, -6.0f, 0u, 0.6f };
    struct StatRec { UInt32 cubes; float fps; };
    static StatRec* sStats = nullptr;
    static UInt32 sStatCount = 0;
    static UInt32 sStatCap = 0;
    static void PushStat(UInt32 cubes, float fps) noexcept {
        if (!sStats) {
            sStatCap = 1024u;
            sStats = reinterpret_cast<StatRec*>(::operator new(sizeof(StatRec) * sStatCap));
            sStatCount = 0u;
        }
        if (sStatCount >= sStatCap) {
            UInt32 nc = sStatCap ? (sStatCap << 1) : 1024u;
            auto* nb = reinterpret_cast<StatRec*>(::operator new(sizeof(StatRec) * nc));
            for (UInt32 i = 0; i < sStatCount; ++i) nb[i] = sStats[i];
            ::operator delete(sStats);
            sStats = nb; sStatCap = nc;
        }
        sStats[sStatCount++] = StatRec{ cubes, fps };
    }
    export void ComputeCube(UInt32 i) noexcept {
        UInt32 cols = gs.cols ? gs.cols : 1u;
        UInt32 ix = (cols > 0u) ? (i % cols) : 0u;
        UInt32 iz = (cols > 0u) ? (i / cols) : 0u;
        float half = static_cast<float>(cols) * 0.5f;
        float x = (static_cast<float>(ix) - half) * gs.sx;
        float y = 0.0f;
        float z = gs.sz + static_cast<float>(iz) * gs.sx;
        float s = gs.scale;
        float sm[16]; Eng::MakeScale(s, s, s, sm);
        float tm[16]; Eng::MakeTranslation(x, y, z, tm);
        float model[16]; Eng::Mul4x4(tm, sm, model);
        float vm[16]; Eng::Mul4x4(gView, model, vm);
        float mvp[16]; Eng::Mul4x4(gProj, vm, mvp);
        for (int k = 0; k < 16; ++k) gCmds[i].mvp[k] = mvp[k];
    }
}

export bool Run() noexcept {
    auto frame = [](Eng::SimpleFrame& fr, UInt32& tick) -> Status {
        Eng::SetClear(fr, 0.06f, 0.06f, 0.09f, 1.0f);

        float aspect = fr.screenW>0.0f && fr.screenH>0.0f ? (fr.screenW/fr.screenH) : 1.3333333f;
        float proj[16]; Eng::MakePerspective(1.3962634f, aspect, 0.05f, 100.0f, proj);

        static bool sInit = false;
        static Prm::TimePoint sLast = 0;
        static float sAngle = 0.0f;
        const float TWO_PI = 6.283185307f;
        const float SPEED = 0.62831853f; // 10s per revolution
        static UInt32 sCubeCount = 2;
        static float sAddRate = 200.0f; // cubes per second
        static bool sLogInit = false;
        if (!sInit) { 
            Prm::Init(); sLast = Prm::Now(); 
            sInit = true; auto cpu = Prm::SystemInfo::Cpu(); 
            UInt32 workers = cpu.logicalCores ? cpu.logicalCores : 4; 
            (void)Sys::JobStart(workers); 
        }
        Prm::TimePoint now = Prm::Now();
        float dt = static_cast<float>(Prm::Delta(sLast, now)) / 1000000000.0f;
        sLast = now;
        sAngle += dt * SPEED;
        if (sAngle >= TWO_PI) sAngle -= TWO_PI;
        if (sAngle < 0.0f) sAngle += TWO_PI;
        float angle = sAngle;
        float fps = (dt > 0.0f) ? (1.0f / dt) : 0.0f;
        float add = sAddRate * dt;
        if (add > 0.0f) { sCubeCount += static_cast<UInt32>(add); }
        float camR = 10.0f;
        float centerX = 0.0f, centerY = 0.0f, centerZ = -6.0f;
        float eyeX = centerX + camR * Eng::FastCos(angle);
        float eyeZ = centerZ + camR * Eng::FastSin(angle);
        float eyeY = centerY + 8.0f;
        float view[16]; Eng::MakeLookAt(eyeX,eyeY,eyeZ, centerX,centerY,centerZ, 0.0f,1.0f,0.0f, view);
        for (int i=0;i<16;++i) { Pjt::gProj[i] = proj[i]; Pjt::gView[i] = view[i]; }

        float rotA[16]; Eng::MakeRotationY(0.0f, rotA);
        float scaleA[16]; Eng::MakeScale(0.8f,0.8f,0.8f, scaleA);
        float transA[16]; Eng::MakeTranslation(-1.0f, 0.0f, -6.0f, transA);
        float rtA[16]; Eng::Mul4x4(rotA, scaleA, rtA);
        float modelA[16]; Eng::Mul4x4(transA, rtA, modelA);
        float vmA[16]; Eng::Mul4x4(view, modelA, vmA);
        float mvpA[16]; Eng::Mul4x4(proj, vmA, mvpA);
        Eng::RGBA faceA[6] = { {1.0f,0.0f,0.0f,1.0f}, {0.0f,1.0f,0.0f,1.0f}, {0.0f,0.0f,1.0f,1.0f}, {1.0f,1.0f,0.0f,1.0f}, {1.0f,0.0f,1.0f,1.0f}, {0.0f,1.0f,1.0f,1.0f} };
        Eng::PushUnitCubeFaceColors(fr, mvpA, faceA);

        float rotB[16]; Eng::MakeRotationY(0.0f, rotB);
        float scaleB[16]; Eng::MakeScale(0.8f,0.8f,0.8f, scaleB);
        float transB[16]; Eng::MakeTranslation(1.0f, 0.0f, -6.0f, transB);
        float rtB[16]; Eng::Mul4x4(rotB, scaleB, rtB);
        float modelB[16]; Eng::Mul4x4(transB, rtB, modelB);
        float vmB[16]; Eng::Mul4x4(view, modelB, vmB);
        float mvpB[16]; Eng::Mul4x4(proj, vmB, mvpB);
        Eng::RGBA faceB[6] = { {0.9f,0.4f,0.1f,1.0f}, {0.4f,0.9f,0.1f,1.0f}, {0.1f,0.4f,0.9f,1.0f}, {0.9f,0.1f,0.4f,1.0f}, {0.4f,0.1f,0.9f,1.0f}, {0.1f,0.9f,0.4f,1.0f} };
        Eng::PushUnitCubeFaceColors(fr, mvpB, faceB);

        UInt32 maxCubes = Eng::kMaxTris / 12u;
        UInt32 renderCubes = sCubeCount > maxCubes ? maxCubes : sCubeCount;
        static float accLog = 0.0f;
        accLog += dt;
        if (renderCubes > 2u) {
            Pjt::gs.cols = static_cast<UInt32>(Eng::FastSqrt(static_cast<float>(renderCubes)) + 1.0f);
            Pjt::gCmds = reinterpret_cast<Pjt::CubeCmd*>(::operator new(sizeof(Pjt::CubeCmd) * renderCubes));
            Pjt::gCount = renderCubes;
            (void)Sys::ParallelFor(0u, renderCubes, 64u, &Pjt::ComputeCube);
            for (UInt32 i=0;i<renderCubes;++i) {
                Eng::PushUnitCubeFaceColors(fr, Pjt::gCmds[i].mvp, faceA);
            }
            ::operator delete(Pjt::gCmds);
        }
        if (accLog >= 1.0f) {
            accLog = 0.0f;
            auto h = Prm::File::Stdout();
            char buf[128]; int n = 0;
            auto putc = [&](char c){ if (n<120) buf[n++]=c; };
            auto puts = [&](const char* s){ while(*s && n<120){ buf[n++]=*s++; } };
            auto putu = [&](UInt32 v){ char t[16]; int k=0; if(v==0){ t[k++]='0'; } else { UInt32 x=v; char d[16]; int p=0; while(x){ d[p++]=char('0'+(x%10)); x/=10; } while(p) t[k++]=d[--p]; } for(int i=0;i<k && n<120;++i) buf[n++]=t[i]; };
            auto putf = [&](float f){ if (f<0){ putc('-'); f=-f; } UInt32 iv = (UInt32)f; float frac = f - (float)iv; putu(iv); putc('.'); UInt32 frac3 = (UInt32)(frac*1000.0f+0.5f); if(frac3<100){ if(frac3<10) { putc('0'); putc('0'); } else { putc('0'); } } putu(frac3); };
            puts("cubes="); putu(sCubeCount); puts(" fps="); putf(fps); puts(" tris="); putu(fr.triCount); putc('\n');
            (void)Prm::File::Write(h, reinterpret_cast<const Byte*>(buf), (USize)n);
            Pjt::PushStat(sCubeCount, fps);
        }

        

        return Ok(StatusDomain::System());
    };
    bool ok = Eng::RunWindowed(1024, 768, frame);
    if (Pjt::sStats && Pjt::sStatCount > 0u) {
        auto h = Prm::File::Stdout();
        const char* header = "cubes,fps\n";
        (void)Prm::File::Write(h, reinterpret_cast<const Byte*>(header), (USize)12);
        for (UInt32 i = 0; i < Pjt::sStatCount; ++i) {
            char buf[128]; int n = 0;
            auto putc = [&](char c){ if (n<120) buf[n++]=c; };
            auto putu = [&](UInt32 v){ char t[16]; int k=0; if(v==0){ t[k++]='0'; } else { UInt32 x=v; char d[16]; int p=0; while(x){ d[p++]=char('0'+(x%10)); x/=10; } while(p) t[k++]=d[--p]; } for(int j=0;j<k && n<120;++j) buf[n++]=t[j]; };
            auto putf = [&](float f){ if (f<0){ putc('-'); f=-f; } UInt32 iv = (UInt32)f; float frac = f - (float)iv; putu(iv); putc('.'); UInt32 frac3 = (UInt32)(frac*1000.0f+0.5f); if(frac3<100){ if(frac3<10) { putc('0'); putc('0'); } else { putc('0'); } } putu(frac3); };
            auto puts = [&](const char* s){ while(*s && n<120){ buf[n++]=*s++; } };
            putu(Pjt::sStats[i].cubes); putc(','); putf(Pjt::sStats[i].fps); putc('\n');
            (void)Prm::File::Write(h, reinterpret_cast<const Byte*>(buf), (USize)n);
        }
        ::operator delete(Pjt::sStats);
        Pjt::sStats = nullptr; Pjt::sStatCount = 0u; Pjt::sStatCap = 0u;
    }
    return ok;
}
