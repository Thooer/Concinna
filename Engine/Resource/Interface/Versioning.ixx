module;
export module Engine.Resource:Versioning;

import Lang;
import Foundation.IO;
import :Handle;

export namespace Engine::Resource {
    struct VersionTag {
        UInt64 value;
        constexpr VersionTag() noexcept : value(0) {}
        constexpr explicit VersionTag(UInt64 v) noexcept : value(v) {}
    };

    [[nodiscard]] UInt64 ComputeContentHash(::Foundation::IO::IInputStream& in) noexcept;

    [[nodiscard]] VersionTag MakeVersionTag(UInt64 seed, UInt64 contentHash, UInt64 metaVersion) noexcept;

    [[nodiscard]] bool ValidateAssetVersion(Handle h, const VersionTag& tag) noexcept;
}