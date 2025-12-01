module;
module Engine.Resource:Versioning;

import Lang;
import Foundation.IO;
import :Handle;

using namespace Engine::Resource;

    [[nodiscard]] inline VersionTag MakeVersionTag(UInt64 seed, Span<const Byte> data) noexcept {
        auto dataHash = Fnv1aBytes(data.data(), data.size());
        return VersionTag{ HashUtils::HashCombine(seed, dataHash) };
    }

    [[nodiscard]] inline bool Verify(const VersionTag& tag, UInt64 seed, Span<const Byte> data) noexcept {
        auto t = MakeVersionTag(seed, data);
        return t.value == tag.value;
    }

    [[nodiscard]] inline bool Verify(const VersionTag& tag, Span<const Byte> data) noexcept {
        constexpr UInt64 kDefaultSeed = 0ull;
        auto t = MakeVersionTag(kDefaultSeed, data);
        return t.value == tag.value;
    }

    [[nodiscard]] inline UInt64 ComputeContentHash(::Foundation::IO::IInputStream& in) noexcept {
        UInt64 h = kFnvOffset;
        Byte buf[8*1024]{};
        for (;;) {
            auto r = in.Read(Span<Byte>(buf, sizeof(buf)));
            if (!r.IsOk()) break;
            auto n = r.OkValue();
            if (n == 0) break;
            UInt64 chunkHash = Fnv1aBytes(buf, static_cast<size_t>(n));
            h = CombineHashes(h, chunkHash);
        }
        return h;
    }

    [[nodiscard]] inline VersionTag MakeVersionTag(UInt64 seed, UInt64 contentHash, UInt64 metaVersion) noexcept {
        auto c = HashUtils::HashCombine(contentHash, metaVersion);
        auto v = HashUtils::HashCombine(seed, c);
        return VersionTag{ v };
    }

    [[nodiscard]] inline bool ValidateAssetVersion(Handle, const VersionTag& tag) noexcept {
        return tag.value != 0ull;
    }
}