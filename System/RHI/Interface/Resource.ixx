module;
export module Sys.RHI:Resource;

import Lang;
import :Types;

export namespace Sys {
    enum class ResourceUsage : UInt8 { Default = 0, Upload = 1, Readback = 2 };

    struct BufferDesc { USize size{0}; ResourceUsage usage{ ResourceUsage::Default }; bool isUAV{ false }; };
    struct TextureDesc { UInt32 width{0}; UInt32 height{0}; Format format{ Format::R8G8B8A8_UNORM }; };

    export Expect<BufferHandle> CreateBuffer(const Device& dev, const BufferDesc& desc) noexcept;
    export Expect<TextureHandle> CreateTexture(const Device& dev, const TextureDesc& desc) noexcept;
}
