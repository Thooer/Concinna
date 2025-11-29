module;
export module Engine.Resource:Versioning;

import Lang;
import Foundation.IO;
import :Handle;

export namespace Engine::Resource {
    struct VersionTag {
        Language::UInt64 value;
        constexpr VersionTag() noexcept : value(0) {}
        constexpr explicit VersionTag(Language::UInt64 v) noexcept : value(v) {}
    };

    [[nodiscard]] Language::UInt64 ComputeContentHash(::Foundation::IO::IInputStream& in) noexcept;

    [[nodiscard]] VersionTag MakeVersionTag(Language::UInt64 seed, Language::UInt64 contentHash, Language::UInt64 metaVersion) noexcept;

    [[nodiscard]] bool ValidateAssetVersion(Handle h, const VersionTag& tag) noexcept;
}