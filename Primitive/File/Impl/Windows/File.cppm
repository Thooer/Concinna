module Platform;

import Prm;
import :File;

using namespace Platform;

Expect<USize> FilePath::Normalize(StringView, Span<Char8, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Expect<USize> FilePath::Join(StringView, StringView, Span<Char8, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
Expect<USize> FilePath::Basename(StringView, Span<Char8, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }

