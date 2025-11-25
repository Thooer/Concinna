module Platform;

import Prm;
import :Debug;

using namespace Platform;

extern "C" __declspec(dllimport) void OutputDebugStringA(const char*);

void Debug::Break() noexcept { }
Status Debug::Output(StringView text) noexcept {
    OutputDebugStringA(reinterpret_cast<const char*>(text.data()));
    return Ok(StatusDomain::System());
}

