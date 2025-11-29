module;
export module Eng.Renderer:Types;

import Lang;
 

export namespace Eng::Renderer {
    struct RenderInstance {
        float model[16]{};
        UInt32 mesh{};
        UInt32 mat{};
    };
}
