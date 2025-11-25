module Platform;

import Prm;
import :Audio;

using namespace Platform;

Expect<USize> Audio::EnumerateDevices(Span<AudioDevice, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }

