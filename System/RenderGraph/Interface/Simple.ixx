module;
export module Sys.RenderGraph:Simple;

import Lang;
import :Types;

export namespace Sys {
    class SimpleRenderer {
    public:
        bool Submit(const RenderInstance* instances, USize count) noexcept { m_count += count; return true; }
        void Flush() noexcept { m_count = 0; }
        void Present() noexcept {}
        USize Submitted() const noexcept { return m_count; }
    private:
        USize m_count{0};
    };
}
