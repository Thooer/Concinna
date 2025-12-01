module;
export module Sys.RHI:Sync;

import Lang;
import :Types;

export namespace Sys {
    enum class PipelineStage : UInt32 { TopOfPipe = 0, ColorOutput = 1 };
    enum class AccessMask : UInt32 { None = 0, ColorWrite = 1 };
    enum class ImageLayout : UInt32 { Undefined = 0, ColorAttachment = 1, PresentSrc = 2 };

    struct Barrier {
        TextureHandle image{};
        PipelineStage srcStage{ PipelineStage::TopOfPipe };
        AccessMask srcAccess{ AccessMask::None };
        ImageLayout oldLayout{ ImageLayout::Undefined };
        PipelineStage dstStage{ PipelineStage::ColorOutput };
        AccessMask dstAccess{ AccessMask::ColorWrite };
        ImageLayout newLayout{ ImageLayout::ColorAttachment };
    };
}
