module;
export module Platform:Audio;

import Prm;

export namespace Platform {
    export struct AudioDevice { UInt32 sampleRate{0}; UInt16 channels{0}; };

    export class Audio {
    public:
        static Expect<USize> EnumerateDevices(Span<AudioDevice, DynamicExtent> out) noexcept;
    };
}

