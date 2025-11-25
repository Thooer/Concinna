module;
export module Prm.Audio;

export namespace Prm {
    export struct AudioDevice { UInt32 sampleRate{0}; UInt16 channels{0}; };

    export class Audio {
    public:
        static Expect<USize> EnumerateDevices(Span<AudioDevice, DynamicExtent> out) noexcept;
    };
}
