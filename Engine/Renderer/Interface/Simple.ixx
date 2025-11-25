module;
export module Engine.Renderer:Simple;

import Language;
import :Types;

export namespace Engine::Renderer {
    class SimpleRenderer {
    public:
        bool Submit(const RenderInstance* instances, Language::USize count) noexcept { m_count += count; return true; }
        void Flush() noexcept { m_count = 0; }
        void Present() noexcept {}
        Language::USize Submitted() const noexcept { return m_count; }
    private:
        Language::USize m_count{0};
    };
}