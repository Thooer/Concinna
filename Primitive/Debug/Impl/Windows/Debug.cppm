module Prm.Debug;

import :Debug;

using namespace Prm;

extern "C" __declspec(dllimport) void OutputDebugStringA(const char*);

void Debug::Break() noexcept { }
Status Debug::Output(StringView text) noexcept {
    OutputDebugStringA(reinterpret_cast<const char*>(text.data()));
    return Ok(StatusDomain::System());
}
