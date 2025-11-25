module;
export module Engine.Renderer:RHI.Types;

import Language;

export namespace Engine::Renderer::RHI {
    enum class QueueType : Language::UInt32 { Graphics = 0, Compute = 1, Transfer = 2, Present = 3 };

    struct DeviceCreateInfo { bool enableValidation{false}; };
    struct SurfaceInfo { void* hwnd{nullptr}; Language::UInt32 width{0}; Language::UInt32 height{0}; };

    struct Device { void* p{nullptr}; };
    struct Swapchain { void* p{nullptr}; };
    struct Queue { void* p{nullptr}; };
    struct Fence { void* p{nullptr}; };

    class CommandList {
    public:
        void Begin() noexcept;
        void End() noexcept;
        void ClearColor(float r, float g, float b, float a) noexcept;
        void DrawTriangle2D(float x0, float y0, float x1, float y1, float x2, float y2, float r, float g, float b, float a) noexcept;
        const float* ClearColorRGBA() const noexcept { return m_clear; }
        bool Recording() const noexcept { return m_recording; }
        friend Language::Status Submit(const Queue&, CommandList&, Fence*, const Swapchain&) noexcept;
    private:
        float m_clear[4]{0.0f,0.0f,0.0f,1.0f};
        bool m_recording{false};
        struct Tri2D { float x0{}, y0{}, x1{}, y1{}, x2{}, y2{}, c0{}, c1{}, c2{}, c3{}; };
        Tri2D m_tris[16]{};
        Language::USize m_triCount{0};
    };
}