module Prm.Clipboard;

import Prm.Clipboard;
import Lang.Element;
import Lang.Flow;

using namespace Prm;

Status Clipboard::SetText(Span<const Char8, DynamicExtent>) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
Expect<USize> Clipboard::GetText(Span<Char8, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
