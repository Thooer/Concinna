module Prm.Debug;

import :Debug;

using namespace Prm;

void Debug::Break() noexcept {}
Status Debug::Output(StringView) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
