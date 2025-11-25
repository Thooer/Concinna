module Platform;

import Prm;
import :Debug;

using namespace Platform;

void Debug::Break() noexcept {}
Status Debug::Output(StringView) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }

