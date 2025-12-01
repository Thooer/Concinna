module;
module Sys.RHI;

import Lang;
import :Types;
import :API;
import Prm.WSI;
import Prm.Window;

namespace Sys {
    Expect<Device> CreateDevice() noexcept { return CreateDevice(DeviceCreateInfo{}); }
    void CommandList::Begin() noexcept { m_recording = true; m_triCount = 0; }
    void CommandList::End() noexcept { m_recording = false; }
    void CommandList::ClearColor(float r, float g, float b, float a) noexcept { m_clear[0]=r; m_clear[1]=g; m_clear[2]=b; m_clear[3]=a; }
    void CommandList::DrawTriangle(float x0,float y0,float z0,
                                   float x1,float y1,float z1,
                                   float x2,float y2,float z2,
                                   float r,float g,float b,float a) noexcept {
        if (m_triCount < static_cast<USize>(16)) {
            auto& t = m_tris[static_cast<size_t>(m_triCount++)];
            t.x0=x0; t.y0=y0; t.z0=z0; t.x1=x1; t.y1=y1; t.z1=z1; t.x2=x2; t.y2=y2; t.z2=z2; t.c0=r; t.c1=g; t.c2=b; t.c3=a;
        }
    }
    void CommandList::DrawTriangle2D(float x0,float y0,float x1,float y1,float x2,float y2,float r,float g,float b,float a) noexcept {
        DrawTriangle(x0,y0,0.5f,x1,y1,0.5f,x2,y2,0.5f,r,g,b,a);
    }

    struct CpuDeviceState { void* alloc{}; };
    struct CpuSwapchainState { void* wsi{}; uint8_t* pixels{}; float* depth{}; uint32_t width{}, height{}; int pitch{}; bool wroteOnce{}; };
    struct CpuQueueState { CpuDeviceState* ds{}; };

    Expect<Device> CreateDevice(const DeviceCreateInfo&) noexcept {
        CpuDeviceState* ds = reinterpret_cast<CpuDeviceState*>(::operator new(sizeof(CpuDeviceState)));
        ds->alloc = nullptr;
        Device d{}; d.p = ds; return Expect<Device>::Ok(d);
    }
    Expect<Swapchain> CreateSwapchain(const Device& dev, const SurfaceInfo& si) noexcept {
        if (!dev.p || si.width==0 || si.height==0) return Expect<Swapchain>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        CpuSwapchainState* ss = reinterpret_cast<CpuSwapchainState*>(::operator new(sizeof(CpuSwapchainState)));
        auto pr = ::Prm::CreateCpuPresent(::Prm::WindowHandle{si.hwnd}, si.width, si.height);
        if (!pr.IsOk()) { ::operator delete(ss); return Expect<Swapchain>::Err(pr.Error()); }
        ss->wsi = pr.Value();
        ss->width = si.width; ss->height = si.height;
        auto br = ::Prm::CpuGetBuffer(ss->wsi);
        if (!br.IsOk()) { ::Prm::DestroyCpuPresent(ss->wsi); ::operator delete(ss); return Expect<Swapchain>::Err(br.Error()); }
        ss->pixels = reinterpret_cast<uint8_t*>(br.Value());
        ss->pitch = static_cast<int>(::Prm::CpuGetPitch(ss->wsi));
        ss->depth = reinterpret_cast<float*>(::operator new(sizeof(float) * ss->width * ss->height));
        Swapchain sc{}; sc.p = ss; return Expect<Swapchain>::Ok(sc);
    }
    Expect<Queue> GetQueue(const Device& dev, QueueType) noexcept {
        if (!dev.p) return Expect<Queue>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        auto ds = reinterpret_cast<CpuDeviceState*>(dev.p);
        CpuQueueState* qs = reinterpret_cast<CpuQueueState*>(::operator new(sizeof(CpuQueueState)));
        qs->ds = ds;
        Queue q{}; q.p = qs; return Expect<Queue>::Ok(q);
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
    static inline void _draw_tri(uint8_t* dst,float* depth,int pitch,uint32_t w,uint32_t h,
                                 float x0,float y0,float z0,
                                 float x1,float y1,float z1,
                                 float x2,float y2,float z2,
                                 uint8_t r,uint8_t g,uint8_t b,uint8_t a) noexcept {
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
                if (_inside(x0,y0,x1,y1,x2,y2,px,py)) {
                    float area = (x1 - x0)*(y2 - y0) - (y1 - y0)*(x2 - x0);
                    float w0 = ((x1 - px)*(y2 - py) - (y1 - py)*(x2 - px)) / area;
                    float w1 = ((x2 - px)*(y0 - py) - (y2 - py)*(x0 - px)) / area;
                    float w2 = 1.0f - w0 - w1;
                    float z = w0*z0 + w1*z1 + w2*z2;
                    int di = y * w + x;
                    if (z < depth[di]) {
                        depth[di] = z;
                        row[x*4+0]=b; row[x*4+1]=g; row[x*4+2]=r; row[x*4+3]=a;
                    }
                }
            }
        }
    }
    Status Submit(const Queue& q, CommandList& cmd, Fence*, const Swapchain& sc) noexcept {
        (void)q;
        if (!sc.p) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        CpuSwapchainState* ss = reinterpret_cast<CpuSwapchainState*>(sc.p);
        if (!ss->pixels) return Err(StatusDomain::System(), StatusCode::Failed);
        uint8_t r = (uint8_t)(cmd.ClearColorRGBA()[0] * 255.0f);
        uint8_t g = (uint8_t)(cmd.ClearColorRGBA()[1] * 255.0f);
        uint8_t b = (uint8_t)(cmd.ClearColorRGBA()[2] * 255.0f);
        uint8_t a = (uint8_t)(cmd.ClearColorRGBA()[3] * 255.0f);
        _clear_u32(reinterpret_cast<uint8_t*>(ss->pixels), ss->pitch, ss->width, ss->height, r,g,b,a);
        for (uint32_t i=0;i<ss->width*ss->height;++i) { ss->depth[i] = 1.0f; }
        for (USize i=0;i<cmd.m_triCount;++i) {
            auto& t = cmd.m_tris[static_cast<size_t>(i)];
            uint8_t tr = (uint8_t)(t.c0*255.0f);
            uint8_t tg = (uint8_t)(t.c1*255.0f);
            uint8_t tb = (uint8_t)(t.c2*255.0f);
            uint8_t ta = (uint8_t)(t.c3*255.0f);
            _draw_tri(reinterpret_cast<uint8_t*>(ss->pixels), ss->depth, ss->pitch, ss->width, ss->height,
                      t.x0,t.y0,t.z0, t.x1,t.y1,t.z1, t.x2,t.y2,t.z2, tr,tg,tb,ta);
        }
        (void)::Prm::CpuPresent(ss->wsi);
        return Ok(StatusDomain::System());
    }
    void DestroySwapchain(const Device& dev, Swapchain& sc) noexcept {
        (void)dev;
        if (!sc.p) return; CpuSwapchainState* ss = reinterpret_cast<CpuSwapchainState*>(sc.p);
        if (ss->wsi) { (void)::Prm::DestroyCpuPresent(ss->wsi); }
        if (ss->depth) { ::operator delete(ss->depth); ss->depth = nullptr; }
        ::operator delete(ss);
        sc.p = nullptr;
    }
    void DestroyDevice(Device& dev) noexcept {
        if (!dev.p) return; CpuDeviceState* ds = reinterpret_cast<CpuDeviceState*>(dev.p);
        ::operator delete(ds);
        dev.p = nullptr;
    }
}
