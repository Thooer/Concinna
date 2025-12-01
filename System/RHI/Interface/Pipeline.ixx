module;
export module Sys.RHI:Pipeline;

import Lang;
import :Types;

export namespace Sys {
    struct PipelineDesc { PipelineHandle base{}; };

    export Expect<PipelineHandle> CreatePipeline(const Device& dev, const PipelineDesc& desc) noexcept;
}
