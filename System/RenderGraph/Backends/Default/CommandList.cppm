module;
module Sys.RenderGraph;

import Lang;
import :CommandList;
import :Simple;

namespace Sys {
    CommandList::CommandList(SimpleRenderer* r) noexcept : m_renderer(r), m_drawCalls(0), m_submissions(0), m_flushes(0), m_recording(false) {}
    void CommandList::Begin() noexcept { m_recording = true; }
    void CommandList::Draw(const RenderInstance* instances, USize count) noexcept {
        if (!m_recording || !m_renderer) return;
        (void)m_renderer->Submit(instances, count);
        m_drawCalls += static_cast<USize>(1);
        m_submissions += count;
    }
    void CommandList::End() noexcept { m_recording = false; }
    void CommandList::Flush() noexcept { if (m_renderer) m_renderer->Flush(); m_flushes += static_cast<USize>(1); }
    USize CommandList::DrawCalls() const noexcept { return m_drawCalls; }
    USize CommandList::Submissions() const noexcept { return m_submissions; }
    USize CommandList::Flushes() const noexcept { return m_flushes; }
}
