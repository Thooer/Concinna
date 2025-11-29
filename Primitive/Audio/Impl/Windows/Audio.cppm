module Prm.Audio;

import :Types;
import :Ops;
import Lang.Element;
import Lang.Flow;

using namespace Prm;

Expect<USize> Audio::EnumerateDevices(Span<AudioDevice, DynamicExtent>) noexcept { return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
