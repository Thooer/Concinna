module;
module Engine.Renderer;

import Language;
import :RHI.Types;
import :RHI.API;
import Platform;
import Foundation.Memory;

namespace Engine::Renderer::RHI {
    void CommandList::Begin() noexcept { m_recording = true; m_triCount = 0; }
    void CommandList::End() noexcept { m_recording = false; }
    void CommandList::ClearColor(float r, float g, float b, float a) noexcept { m_clear[0]=r; m_clear[1]=g; m_clear[2]=b; m_clear[3]=a; }
    void CommandList::DrawTriangle2D(float x0,float y0,float x1,float y1,float x2,float y2,float r,float g,float b,float a) noexcept {
        if (m_triCount < static_cast<Language::USize>(16)) {
            auto& t = m_tris[static_cast<size_t>(m_triCount++)];
            t.x0=x0; t.y0=y0; t.x1=x1; t.y1=y1; t.x2=x2; t.y2=y2; t.c0=r; t.c1=g; t.c2=b; t.c3=a;
        }
    }

    struct CpuDeviceState { ::Foundation::Memory::IAllocator* alloc{}; };
    struct CpuSwapchainState { void* wsi{}; uint8_t* pixels{}; uint32_t width{}, height{}; int pitch{}; bool wroteOnce{}; };
    struct CpuQueueState { CpuDeviceState* ds{}; };

    Language::StatusResult<Device> CreateDevice(const DeviceCreateInfo&) noexcept {
        auto ar = ::Foundation::Memory::CreateDefaultAllocator();
        if (!ar.IsOk()) return Language::StatusResult<Device>::Err(ar.ErrStatus());
        auto alloc = ar.OkValue();
        auto mr = alloc->Allocate(static_cast<Language::USize>(sizeof(CpuDeviceState)), static_cast<Language::USize>(alignof(CpuDeviceState)));
        if (!mr.IsOk()) { ::Foundation::Memory::DestroyRuntimeAllocator(alloc); return Language::StatusResult<Device>::Err(mr.ErrStatus()); }
        CpuDeviceState* ds = reinterpret_cast<CpuDeviceState*>(mr.OkValue());
        ds->alloc = alloc;
        Device d{}; d.p = ds; return Language::StatusResult<Device>::Ok(d);
    }
    Language::StatusResult<Swapchain> CreateSwapchain(const Device& dev, const SurfaceInfo& si) noexcept {
        if (!dev.p || si.width==0 || si.height==0) return Language::StatusResult<Swapchain>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::InvalidArgument));
        auto ds = reinterpret_cast<CpuDeviceState*>(dev.p);
        auto mr = ds->alloc->Allocate(static_cast<Language::USize>(sizeof(CpuSwapchainState)), static_cast<Language::USize>(alignof(CpuSwapchainState)));
        if (!mr.IsOk()) return Language::StatusResult<Swapchain>::Err(mr.ErrStatus());
        CpuSwapchainState* ss = reinterpret_cast<CpuSwapchainState*>(mr.OkValue());
        auto pr = ::Platform::WSI::CreateCpuPresent(::Platform::WindowHandle{si.hwnd}, si.width, si.height);
        if (!pr.IsOk()) { ds->alloc->Deallocate(reinterpret_cast<Language::Byte*>(ss), static_cast<Language::USize>(sizeof(CpuSwapchainState)), static_cast<Language::USize>(alignof(CpuSwapchainState))); return Language::StatusResult<Swapchain>::Err(pr.ErrStatus()); }
        ss->wsi = pr.OkValue();
        auto br = ::Platform::WSI::CpuGetBuffer(ss->wsi);
        if (!br.IsOk()) { ::Platform::WSI::DestroyCpuPresent(ss->wsi); ds->alloc->Deallocate(reinterpret_cast<Language::Byte*>(ss), static_cast<Language::USize>(sizeof(CpuSwapchainState)), static_cast<Language::USize>(alignof(CpuSwapchainState))); return Language::StatusResult<Swapchain>::Err(br.ErrStatus()); }
        ss->pixels = reinterpret_cast<uint8_t*>(br.OkValue());
        ss->width = si.width; ss->height = si.height; ss->pitch = static_cast<int>(::Platform::WSI::CpuGetPitch(ss->wsi));
        Swapchain sc{}; sc.p = ss; return Language::StatusResult<Swapchain>::Ok(sc);
    }
    Language::StatusResult<Queue> GetQueue(const Device& dev, QueueType) noexcept {
        if (!dev.p) return Language::StatusResult<Queue>::Err(Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::InvalidArgument));
        auto ds = reinterpret_cast<CpuDeviceState*>(dev.p);
        auto mr = ds->alloc->Allocate(static_cast<Language::USize>(sizeof(CpuQueueState)), static_cast<Language::USize>(alignof(CpuQueueState)));
        if (!mr.IsOk()) return Language::StatusResult<Queue>::Err(mr.ErrStatus());
        CpuQueueState* qs = reinterpret_cast<CpuQueueState*>(mr.OkValue());
        qs->ds = ds;
        Queue q{}; q.p = qs; return Language::StatusResult<Queue>::Ok(q);
    }
    static inline void _clear_u32(uint8_t* dst, int pitch, uint32_t w, uint32_t h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
        for (uint32_t y=0;y<h;++y) {
            uint8_t* row = dst + y * pitch;
            for (uint32_t x=0;x<w;++x) {
                row[x*4+0] = b;
                row[x*4+1] = g;
                row[x*4+2] = r;
                row[x*4+3] = a;
            }
        }
    }
    static inline bool _inside(float ax,float ay,float bx,float by,float cx,float cy,float px,float py) noexcept {
        float abx = bx - ax, aby = by - ay;
        float bcx = cx - bx, bcy = cy - by;
        float cax = ax - cx, cay = ay - cy;
        float apx = px - ax, apy = py - ay;
        float bpx = px - bx, bpy = py - by;
        float cpx = px - cx, cpy = py - cy;
        float e0 = abx*apy - aby*apx;
        float e1 = bcx*bpy - bcy*bpx;
        float e2 = cax*cpy - cay*cpx;
        return (e0>=0 && e1>=0 && e2>=0) || (e0<=0 && e1<=0 && e2<=0);
    }
    static inline void _draw_tri(uint8_t* dst,int pitch,uint32_t w,uint32_t h,float x0,float y0,float x1,float y1,float x2,float y2,uint8_t r,uint8_t g,uint8_t b,uint8_t a) noexcept {
        float minx = x0; if (x1<minx) minx=x1; if (x2<minx) minx=x2;
        float maxx = x0; if (x1>maxx) maxx=x1; if (x2>maxx) maxx=x2;
        float miny = y0; if (y1<miny) miny=y1; if (y2<miny) miny=y2;
        float maxy = y0; if (y1>maxy) maxy=y1; if (y2>maxy) maxy=y2;
        int ix0 = (int)(minx<0?0:minx); int ix1 = (int)(maxx>w-1? (int)w-1 : maxx);
        int iy0 = (int)(miny<0?0:miny); int iy1 = (int)(maxy>h-1? (int)h-1 : maxy);
        for (int y=iy0; y<=iy1; ++y) {
            uint8_t* row = dst + y * pitch;
            for (int x=ix0; x<=ix1; ++x) {
                float px = (float)x + 0.5f; float py = (float)y + 0.5f;
                if (_inside(x0,y0,x1,y1,x2,y2,px,py)) { row[x*4+0]=b; row[x*4+1]=g; row[x*4+2]=r; row[x*4+3]=a; }
            }
        }
    }
    Language::Status Submit(const Queue& q, CommandList& cmd, Fence*, const Swapchain& sc) noexcept {
        (void)q;
        if (!sc.p) return Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::InvalidArgument);
        CpuSwapchainState* ss = reinterpret_cast<CpuSwapchainState*>(sc.p);
        if (!ss->pixels) return Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed);
        uint8_t r = (uint8_t)(cmd.ClearColorRGBA()[0] * 255.0f);
        uint8_t g = (uint8_t)(cmd.ClearColorRGBA()[1] * 255.0f);
        uint8_t b = (uint8_t)(cmd.ClearColorRGBA()[2] * 255.0f);
        uint8_t a = (uint8_t)(cmd.ClearColorRGBA()[3] * 255.0f);
        _clear_u32(reinterpret_cast<uint8_t*>(ss->pixels), ss->pitch, ss->width, ss->height, r,g,b,a);
        for (Language::USize i=0;i<cmd.m_triCount;++i) {
            auto& t = cmd.m_tris[static_cast<size_t>(i)];
            uint8_t tr = (uint8_t)(t.c0*255.0f);
            uint8_t tg = (uint8_t)(t.c1*255.0f);
            uint8_t tb = (uint8_t)(t.c2*255.0f);
            uint8_t ta = (uint8_t)(t.c3*255.0f);
            _draw_tri(reinterpret_cast<uint8_t*>(ss->pixels), ss->pitch, ss->width, ss->height, t.x0,t.y0,t.x1,t.y1,t.x2,t.y2, tr,tg,tb,ta);
        }
        (void)::Platform::WSI::CpuPresent(ss->wsi);
        return Language::Ok(Language::StatusDomain::Platform());
    }
    void DestroySwapchain(const Device& dev, Swapchain& sc) noexcept {
        (void)dev;
        if (!sc.p) return; CpuSwapchainState* ss = reinterpret_cast<CpuSwapchainState*>(sc.p);
        if (ss->wsi) { (void)::Platform::WSI::DestroyCpuPresent(ss->wsi); }
        auto ds = reinterpret_cast<CpuDeviceState*>(dev.p);
        if (ds && ds->alloc) ds->alloc->Deallocate(reinterpret_cast<Language::Byte*>(ss), static_cast<Language::USize>(sizeof(CpuSwapchainState)), static_cast<Language::USize>(alignof(CpuSwapchainState)));
        sc.p = nullptr;
    }
void DestroyDevice(Device& dev) noexcept {
        if (!dev.p) return; CpuDeviceState* ds = reinterpret_cast<CpuDeviceState*>(dev.p);
        auto alloc = ds ? ds->alloc : nullptr;
        if (alloc) {
            alloc->Deallocate(reinterpret_cast<Language::Byte*>(ds), static_cast<Language::USize>(sizeof(CpuDeviceState)), static_cast<Language::USize>(alignof(CpuDeviceState)));
            ::Foundation::Memory::DestroyRuntimeAllocator(alloc);
        }
        dev.p = nullptr;
    }
}