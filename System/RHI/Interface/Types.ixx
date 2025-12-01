module;
export module Sys.RHI:Types;

import Lang;
import Lang.Paradigm;

export namespace Sys {
    enum class Format : UInt32 { R8G8B8A8_UNORM = 37u };
    enum class ShaderStage : UInt32 { Vertex = 0, Fragment = 1, Compute = 2 };
    enum class QueueType : UInt32 { Graphics = 0, Compute = 1, Transfer = 2, Present = 3 };

    struct DeviceCreateInfo { bool enableValidation{false}; };
    struct SurfaceInfo { void* hwnd{nullptr}; UInt32 width{0}; UInt32 height{0}; };

    struct Device { void* p{nullptr}; };
    struct Swapchain { void* p{nullptr}; };
    struct Queue { void* p{nullptr}; };
    struct Fence { void* p{nullptr}; };

    class CommandList {
    public:
        void Begin() noexcept;
        void End() noexcept;
        void ClearColor(float r, float g, float b, float a) noexcept;
        void DrawTriangle(float x0, float y0, float z0,
                          float x1, float y1, float z1,
                          float x2, float y2, float z2,
                          float r, float g, float b, float a) noexcept;
        void DrawTriangle2D(float x0, float y0, float x1, float y1, float x2, float y2, float r, float g, float b, float a) noexcept;
        const float* ClearColorRGBA() const noexcept { return m_clear; }
        bool Recording() const noexcept { return m_recording; }
        friend Status Submit(const Queue&, CommandList&, Fence*, const Swapchain&) noexcept;
    private:
        float m_clear[4]{0.0f,0.0f,0.0f,1.0f};
        bool m_recording{false};
        struct Tri { float x0{}, y0{}, z0{}, x1{}, y1{}, z1{}, x2{}, y2{}, z2{}, c0{}, c1{}, c2{}, c3{}; };
        Tri m_tris[16]{};
        USize m_triCount{0};
    };
}
    struct BufferTag; using BufferHandle = StrongAlias<void*, BufferTag>;
    struct TextureTag; using TextureHandle = StrongAlias<void*, TextureTag>;
    struct PipelineTag; using PipelineHandle = StrongAlias<void*, PipelineTag>;
