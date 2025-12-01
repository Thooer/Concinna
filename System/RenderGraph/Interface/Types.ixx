module;
export module Sys.RenderGraph:Types;

import Lang;

export namespace Sys {
    struct RenderInstance {
        float model[16]{};
        UInt32 mesh{};
        UInt32 mat{};
    };
}
