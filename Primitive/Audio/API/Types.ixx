export module Prm.Audio:Types;
import Lang.Element;

export namespace Prm {
    export struct AudioDevice { UInt32 sampleRate{0}; UInt16 channels{0}; };
}
