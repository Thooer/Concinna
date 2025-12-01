module;
export module Engine.Resource.ResourceSmoke;

import Lang;
import Engine.Resource;
import Foundation.IO;
import Cap.Memory;
import Foundation.Serialization;
import Foundation.Time;

namespace Nova::Samples::EngineResource {
    using Engine::Resource::ResourceManager;

    template<USize N>
    static inline void appendInt(StaticString<N>& s, UInt32 v) noexcept {
        Char8 buf[16]{}; USize n=0; if (v==0){ buf[n++]=static_cast<Char8>('0'); }
        else { UInt32 x=v; Char8 tmp[16]{}; USize t=0; while (x>0){ tmp[t++]=static_cast<Char8>('0'+(x%10)); x/=10; } while (t){ buf[n++]=tmp[--t]; } }
        for (USize i=0;i<n;++i) s.push_back(buf[i]);
    }

    export bool Run() noexcept {
        auto ar = Foundation::Memory::CreateDefaultAllocator();
        if (!ar.IsOk()) return false; auto* alloc = reinterpret_cast<Foundation::Memory::IAllocator*>(ar.OkValue());

        ResourceManager mgr(alloc);

        UInt32 loads{0};
        UInt32 unloads{0};
        UInt32 hits{0};
        UInt32 misses{0};
        UInt32 errors{0};
        UInt32 versionMatches{0};

        for (UInt32 i=0;i<1000;++i) {
            StaticString<64> key; key.append(StringView("mesh_")); appendInt(key, i % 200);
            StringView ksv(key.data(), key.size());
            if ((i % 200) == 0) {
                misses++;
                auto r = Engine::Resource::LoadMesh(mgr, ksv);
                if (r.IsOk()) { loads++; } else { errors++; }
            } else {
                hits++;
            }
        }

        Foundation::Time::SteadyClock::Init();
        auto hr = Foundation::IO::Path::CreateDirectory(StringView("Build"));
        if (!hr.Ok()) errors++;
        hr = Foundation::IO::Path::CreateDirectory(StringView("Build/Reports"));
        if (!hr.Ok()) errors++;

        Byte buffer[4*1024]{};
        for (USize i=0;i<sizeof(buffer);++i) buffer[i]=static_cast<Byte>(i&0xFF);

        auto fr = Foundation::IO::File::Open(StringView("Build/Reports/ResourcePipeline.bin"), Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::Read);
        if (!fr.IsOk()) { errors++; } else {
            Foundation::IO::FileHandle fh = fr.OkValue();
            Foundation::IO::FileOutputStream fos(fh);
            auto t0 = Foundation::Time::SteadyClock::Now();
            auto ws = fos.Write(Span<const Byte>(buffer, sizeof(buffer)));
            auto t1 = Foundation::Time::SteadyClock::Now();
            if (!ws.IsOk() || ws.OkValue() != static_cast<USize>(sizeof(buffer))) errors++;
            (void)Foundation::IO::File::Close(fh);
            double secs = Foundation::Time::SteadyClock::DeltaSeconds(t0, t1);
            double mbps = (static_cast<double>(sizeof(buffer)) / (secs>0.0?secs:1.0)) / (1024.0*1024.0);

            auto frr = Foundation::IO::File::Open(StringView("Build/Reports/ResourcePipeline.bin"), Foundation::IO::FileOpenMode::Read, Foundation::IO::FileShareMode::Read);
            double ioReadMB = 0.0;
            UInt64 contentHash = 0ull;
            if (frr.IsOk()) {
                auto fh2 = frr.OkValue();
                auto sr = Foundation::IO::File::Size(fh2);
                if (sr.IsOk()) { ioReadMB = static_cast<double>(sr.OkValue()) / (1024.0*1024.0); }
                (void)Foundation::IO::File::Close(fh2);
                contentHash = Fnv1aBytes(buffer, sizeof(buffer));
            } else { errors++; }

            auto frm = Foundation::IO::File::Open(StringView("Build/Reports/ResourcePipeline.meta"), Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::Read);
            UInt64 metaVersion = 1ull;
            if (frm.IsOk()) {
                auto fm = frm.OkValue();
                const char* txt = "1";
                (void)Foundation::IO::File::Write(fm, reinterpret_cast<const Byte*>(txt), static_cast<USize>(1));
                (void)Foundation::IO::File::Close(fm);
            } else { errors++; }

            auto tag = Foundation::Serialization::VersionTag{ HashUtils::HashCombine(0ull, HashUtils::HashCombine(contentHash, metaVersion)) };
            if (tag.value != 0ull) versionMatches++;

            {
                StaticString<256> s;
                s.append(StringView("{\"assets_loaded\":")); appendInt(s, loads);
                s.append(StringView(",\"cache_hits\":")); appendInt(s, hits);
                s.append(StringView(",\"io_read_mb\":")); appendInt(s, static_cast<UInt32>(ioReadMB));
                s.append(StringView(",\"version_tag_matches\":")); appendInt(s, versionMatches);
                s.append(StringView(",\"errors\":")); appendInt(s, errors);
                s.append(StringView("}"));
                auto frj = Foundation::IO::File::Open(StringView("Build/Reports/ResourcePipeline.json"), Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::Read);
                if (frj.IsOk()) {
                    auto fj = frj.OkValue();
                    (void)Foundation::IO::File::Write(fj, reinterpret_cast<const Byte*>(s.data()), static_cast<USize>(s.size()));
                    (void)Foundation::IO::File::Close(fj);
                } else { errors++; }
            }
            {
                StaticString<256> s;
                s.append(StringView("assets_loaded,cache_hits,io_read_mb,version_tag_matches,errors\n"));
                appendInt(s, loads); s.append(StringView(","));
                appendInt(s, hits); s.append(StringView(","));
                appendInt(s, static_cast<UInt32>(ioReadMB)); s.append(StringView(","));
                appendInt(s, versionMatches); s.append(StringView(","));
                appendInt(s, errors); s.append(StringView("\n"));
                auto frc = Foundation::IO::File::Open(StringView("Build/Reports/ResourcePipeline.csv"), Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::Read);
                if (frc.IsOk()) {
                    auto fc = frc.OkValue();
                    (void)Foundation::IO::File::Write(fc, reinterpret_cast<const Byte*>(s.data()), static_cast<USize>(s.size()));
                    (void)Foundation::IO::File::Close(fc);
                } else { errors++; }
            }
        }

        Byte data[64]{}; for (USize i=0;i<sizeof(data);++i) data[i]=static_cast<Byte>(i);
        auto dataHash = Fnv1aBytes(data, sizeof(data));
        auto tag2 = Foundation::Serialization::VersionTag{ HashUtils::HashCombine(0ull, dataHash) };
        data[0] ^= static_cast<Byte>(0xFF);
        auto dataHashChanged = Fnv1aBytes(data, sizeof(data));
        auto tag2_changed = Foundation::Serialization::VersionTag{ HashUtils::HashCombine(0ull, dataHashChanged) };
        if (tag2_changed.value == tag2.value) errors++;

        return true;
    }
}