module;
export module Engine.Renderer:CommandList;

import Language;
import :Types;
import :Simple;

export namespace Engine::Renderer {
    class CommandList {
    public:
        explicit CommandList(SimpleRenderer* r) noexcept;
        void Begin() noexcept;
        void Draw(const RenderInstance* instances, Language::USize count) noexcept;
        void End() noexcept;
        void Flush() noexcept;
        Language::USize DrawCalls() const noexcept;
        Language::USize Submissions() const noexcept;
        Language::USize Flushes() const noexcept;
    private:
        SimpleRenderer* m_renderer{};
        Language::USize m_drawCalls{0};
        Language::USize m_submissions{0};
        Language::USize m_flushes{0};
        bool m_recording{false};
    };
}