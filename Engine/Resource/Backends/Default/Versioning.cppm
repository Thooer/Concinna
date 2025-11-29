module;
module Engine.Resource:Versioning;

import Lang;
import Foundation.IO;
import :Handle;

using namespace Engine::Resource;

    [[nodiscard]] inline VersionTag MakeVersionTag(Language::UInt64 seed, Language::Span<const Language::Byte> data) noexcept {
        auto dataHash = Language::Fnv1aBytes(data.data(), data.size());
        return VersionTag{ Language::HashUtils::HashCombine(seed, dataHash) };
    }

    [[nodiscard]] inline bool Verify(const VersionTag& tag, Language::UInt64 seed, Language::Span<const Language::Byte> data) noexcept {
        auto t = MakeVersionTag(seed, data);
        return t.value == tag.value;
    }

    [[nodiscard]] inline bool Verify(const VersionTag& tag, Language::Span<const Language::Byte> data) noexcept {
        constexpr Language::UInt64 kDefaultSeed = 0ull;
        auto t = MakeVersionTag(kDefaultSeed, data);
        return t.value == tag.value;
    }

    [[nodiscard]] inline Language::UInt64 ComputeContentHash(::Foundation::IO::IInputStream& in) noexcept {
        Language::UInt64 h = Language::kFnvOffset;
        Language::Byte buf[8*1024]{};
        for (;;) {
            auto r = in.Read(Language::Span<Language::Byte>(buf, sizeof(buf)));
            if (!r.IsOk()) break;
            auto n = r.OkValue();
            if (n == 0) break;
            Language::UInt64 chunkHash = Language::Fnv1aBytes(buf, static_cast<size_t>(n));
            h = Language::CombineHashes(h, chunkHash);
        }
        return h;
    }

    [[nodiscard]] inline VersionTag MakeVersionTag(Language::UInt64 seed, Language::UInt64 contentHash, Language::UInt64 metaVersion) noexcept {
        auto c = Language::HashUtils::HashCombine(contentHash, metaVersion);
        auto v = Language::HashUtils::HashCombine(seed, c);
        return VersionTag{ v };
    }

    [[nodiscard]] inline bool ValidateAssetVersion(Handle, const VersionTag& tag) noexcept {
        return tag.value != 0ull;
    }
}