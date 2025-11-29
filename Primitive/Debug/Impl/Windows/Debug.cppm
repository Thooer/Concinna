module Prm.Debug;

import Prm.Debug;
import Lang.Element;
import Lang.Flow;

using namespace Prm;

extern "C" __declspec(dllimport) void OutputDebugStringA(const char*);

void Debug::Break() noexcept { }
Status Debug::Output(Span<const Char8, DynamicExtent> text) noexcept {
    OutputDebugStringA(reinterpret_cast<const char*>(text.data()));
    return Ok(StatusDomain::System());
}
