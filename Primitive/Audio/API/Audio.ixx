module;
export module Prm.Audio;
import Prm.Element;
import Prm.Flow;
import Prm.Text;

export namespace Prm {
    export struct AudioDevice { UInt32 sampleRate{0}; UInt16 channels{0}; };

    export class Audio {
    public:
        static Expect<USize> EnumerateDevices(Span<AudioDevice, DynamicExtent> out) noexcept;
    };
}
