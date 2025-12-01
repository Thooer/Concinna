module;
export module Eng.Renderer:Runner;

import Lang;
import Prm.Window;
import Prm.Time;
import Sys.RHI;
import :Types;

export namespace Eng {
    using FrameFunc = Status(*)(SimpleFrame&, UInt32&);
    export bool RunWindowed(UInt32 width, UInt32 height, FrameFunc f) noexcept;
}
