module;
export module Sys.Test:Kit;

import Lang;
import Prm.Window;
import Prm.Time;
import Sys.RHI;

export namespace Sys::Test {
    using FrameFunc = Status(*)(const Sys::Device&, const Sys::Queue&, const Sys::Swapchain&, Sys::CommandList&, UInt32&);
    export bool RunRHIWindowed(UInt32 width, UInt32 height, FrameFunc f) noexcept;
}
