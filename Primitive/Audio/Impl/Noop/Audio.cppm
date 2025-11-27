module Prm.Audio;

import :Types;
import :Ops;

using namespace Prm;

Expect<USize> Audio::EnumerateDevices(Span<AudioDevice, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
