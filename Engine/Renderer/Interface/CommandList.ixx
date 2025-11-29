module;
export module Engine.Renderer:CommandList;

import Lang;
import :Types;
import :Simple;

export namespace Engine::Renderer {
    class CommandList {
    public:
        explicit CommandList(SimpleRenderer* r) noexcept;
        void Begin() noexcept;
        void Draw(const RenderInstance* instances, USize count) noexcept;
        void End() noexcept;
        void Flush() noexcept;
        USize DrawCalls() const noexcept;
        USize Submissions() const noexcept;
        USize Flushes() const noexcept;
    private:
        SimpleRenderer* m_renderer{};
        USize m_drawCalls{0};
        USize m_submissions{0};
        USize m_flushes{0};
        bool m_recording{false};
    };
}
