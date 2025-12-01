module;
export module Eng.Renderer.RHISimpleTriangle;

import Lang;
import Lang.Text;
import Eng.Renderer;
 

static volatile bool g_quit = false;
static Int64 WndProc(void* h, UInt32 m, UIntPtr, IntPtr) { if (m==0x0010u || m==0x0002u) { g_quit=true; } return 0; }

static Status Frame(Eng::SimpleFrame& fr, UInt32& tick) noexcept {
    Eng::SetClear(fr,0.05f,0.05f,0.08f,1.0f);
    float rot[16]; Eng::MakeRotationY(static_cast<float>(tick)*0.02f, rot);
    float trans[16]; Eng::MakeTranslation(0.0f, 0.0f, 4.0f, trans);
    float model[16]; Eng::Mul4x4(rot, trans, model);
    float aspect = fr.screenW>0.0f && fr.screenH>0.0f ? (fr.screenW/fr.screenH) : 1.3333333f;
    float proj[16]; Eng::MakePerspective(1.0471976f, aspect, 0.1f, 100.0f, proj);
    float view[16]; Eng::MakeIdentity(view);
    float mv[16]; Eng::Mul4x4(view, model, mv);
    float mvp[16]; Eng::Mul4x4(mv, proj, mvp);
    Eng::PushUnitCube(fr, mvp, 1.0f, 0.25f, 0.1f, 1.0f);
    return Ok(StatusDomain::System());
}

export bool Run() noexcept { return Eng::RunWindowed(800, 600, &Frame); }
