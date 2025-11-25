module;
export module Engine.Renderer:Types;

import Language;
 

export namespace Engine::Renderer {
    struct RenderInstance {
        float model[16]{};
        Language::UInt32 mesh{};
        Language::UInt32 mat{};
    };
}