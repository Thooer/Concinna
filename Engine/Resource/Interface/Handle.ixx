module;
export module Engine.Resource:Handle;

import Lang;

export namespace Engine::Resource {
    struct ResourceIdTag {};
    using ResourceId = StrongAlias<UInt64, ResourceIdTag>;
    using Handle = UInt32;
    struct MeshHandle { Handle id{0}; };
    struct MaterialHandle { Handle id{0}; };
    struct TextureHandle { Handle id{0}; };
}