export module Prm.Audio:Ops;
import Lang.Element;
import Lang.Flow;
import :Types;

export namespace Prm {
    export class Audio {
    public:
        static Expect<USize> EnumerateDevices(Span<AudioDevice, DynamicExtent> out) noexcept;
    };
}
