module Prm.File;

import Prm.File;
import Element;
import Flow;

using namespace Prm;

Expect<USize> FilePath::Normalize(Span<const Char8, DynamicExtent>, Span<Char8, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Expect<USize> FilePath::Join(Span<const Char8, DynamicExtent>, Span<const Char8, DynamicExtent>, Span<Char8, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Expect<USize> FilePath::Basename(Span<const Char8, DynamicExtent>, Span<Char8, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
