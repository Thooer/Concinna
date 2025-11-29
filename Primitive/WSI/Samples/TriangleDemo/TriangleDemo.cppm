import Prm.Window;
import Prm.WSI;
import Prm.Time;
import Lang.Element;
import Lang.Text;

using Prm::CreateCpuPresent;
using Prm::DestroyCpuPresent;
using Prm::CpuGetBuffer;
using Prm::CpuGetPitch;
using Prm::CpuPresent;

static volatile bool g_quit = false;

static Int64 OnWndProc(void* hWnd, UInt32 msg, UIntPtr, IntPtr) {
    if (msg == 0x0010u || msg == 0x0002u) { g_quit = true; return 0; }
    return 0;
}

static inline void putPixel(void* pixels, UInt32 pitch, UInt32 x, UInt32 y, UInt8 r, UInt8 g, UInt8 b) {
    auto* row = reinterpret_cast<UInt8*>(pixels) + static_cast<USize>(y) * static_cast<USize>(pitch);
    auto* p = row + static_cast<USize>(x) * 4u;
    p[0] = b; p[1] = g; p[2] = r; p[3] = 255u;
}

static inline Int64 edge(Int64 ax, Int64 ay, Int64 bx, Int64 by, Int64 px, Int64 py) {
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

extern "C" int main() {
    Prm::WindowDesc wd{}; wd.width = 800u; wd.height = 600u; wd.visible = true; wd.resizable = true;
    auto w = Prm::Window::Create(wd, &OnWndProc);
    if (!w.IsOk()) return 1;
    auto wh = w.Value();
    (void)Prm::Window::SetTitle(wh, StringView("Triangle"));

    auto ps = Prm::CreateCpuPresent(wh, wd.width, wd.height);
    if (!ps.IsOk()) return 2;
    void* state = ps.Value();
    auto bufExp = Prm::CpuGetBuffer(state);
    if (!bufExp.IsOk()) { (void)Prm::DestroyCpuPresent(state); Prm::Window::Destroy(wh); return 3; }
    void* pixels = bufExp.Value();
    UInt32 pitch = Prm::CpuGetPitch(state);

    Int64 x0 = static_cast<Int64>(wd.width / 2u);
    Int64 y0 = static_cast<Int64>(wd.height / 6u);
    Int64 x1 = static_cast<Int64>(wd.width / 6u);
    Int64 y1 = static_cast<Int64>(wd.height * 5u / 6u);
    Int64 x2 = static_cast<Int64>(wd.width * 5u / 6u);
    Int64 y2 = static_cast<Int64>(wd.height * 5u / 6u);

    Int64 minX = x0; if (x1 < minX) minX = x1; if (x2 < minX) minX = x2; if (minX < 0) minX = 0;
    Int64 maxX = x0; if (x1 > maxX) maxX = x1; if (x2 > maxX) maxX = x2; if (maxX >= static_cast<Int64>(wd.width)) maxX = static_cast<Int64>(wd.width) - 1;
    Int64 minY = y0; if (y1 < minY) minY = y1; if (y2 < minY) minY = y2; if (minY < 0) minY = 0;
    Int64 maxY = y0; if (y1 > maxY) maxY = y1; if (y2 > maxY) maxY = y2; if (maxY >= static_cast<Int64>(wd.height)) maxY = static_cast<Int64>(wd.height) - 1;

    while (!g_quit) {
        bool processed = true; while (processed) { processed = Prm::Window::ProcessOneMessage(wh); }
        for (UInt32 y = 0u; y < wd.height; ++y) {
            auto* row = reinterpret_cast<UInt8*>(pixels) + static_cast<USize>(y) * static_cast<USize>(pitch);
            for (UInt32 x = 0u; x < wd.width; ++x) {
                auto* p = row + static_cast<USize>(x) * 4u;
                p[0] = 8u; p[1] = 8u; p[2] = 8u; p[3] = 255u;
            }
        }
        for (Int64 y = minY; y <= maxY; ++y) {
            for (Int64 x = minX; x <= maxX; ++x) {
                Int64 ex0 = edge(x0, y0, x1, y1, x, y);
                Int64 ex1 = edge(x1, y1, x2, y2, x, y);
                Int64 ex2 = edge(x2, y2, x0, y0, x, y);
                bool hasNeg = (ex0 < 0) || (ex1 < 0) || (ex2 < 0);
                bool hasPos = (ex0 > 0) || (ex1 > 0) || (ex2 > 0);
                if (!(hasNeg && hasPos)) {
                    putPixel(pixels, pitch, static_cast<UInt32>(x), static_cast<UInt32>(y), 255u, 64u, 32u);
                }
            }
        }
        (void)Prm::CpuPresent(state);
        Prm::SleepMs(16u);
    }

    (void)Prm::DestroyCpuPresent(state);
    Prm::Window::Destroy(wh);
    return 0;
}
