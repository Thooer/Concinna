module Prm.Clipboard;

import :Clipboard;

using namespace Prm;

Status Clipboard::SetText(StringView) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
Expect<USize> Clipboard::GetText(Span<Char8, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
