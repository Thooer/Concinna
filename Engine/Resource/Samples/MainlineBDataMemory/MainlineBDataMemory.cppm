module;
export module Engine.Resource.MainlineBDataMemory;

import Lang;
import Foundation.Time;
import Foundation.Containers;
import Foundation.Memory;
import Foundation.IO;
import Engine.Resource;

namespace Nova::Samples::MainlineBDataMemory {
    using ::Foundation::Memory::IAllocator;
    using ::Foundation::Memory::CreateDefaultAllocator;
    using ::Foundation::Containers::SmallVector;
    using ::Foundation::Containers::HashMap;

    static inline double bench_smallvector_inserts(IAllocator* a, Language::USize n) noexcept {
        SmallVector<Language::Byte, 256> sv(a);
        ::Foundation::Time::SteadyClock::Init();
        auto t0 = ::Foundation::Time::SteadyClock::Now();
        for (Language::USize i=0;i<n;++i) { sv.push_back(static_cast<Language::Byte>(i)); }
        auto t1 = ::Foundation::Time::SteadyClock::Now();
        double s = ::Foundation::Time::SteadyClock::DeltaSeconds(t0, t1);
        double ops = (s>0.0) ? (static_cast<double>(n)/s) : 0.0;
        return ops;
    }

    static inline double bench_hashmap_reads(IAllocator* a, Language::USize items, Language::USize queries) noexcept {
        HashMap<Language::UInt64, Language::UInt64> hm(a, static_cast<Language::USize>(items*2));
        for (Language::USize i=0;i<items;++i) { hm.insert(static_cast<Language::UInt64>(i), static_cast<Language::UInt64>(i*3+1)); }
        ::Foundation::Time::SteadyClock::Init();
        auto t0 = ::Foundation::Time::SteadyClock::Now();
        Language::UInt64 out{};
        for (Language::USize i=0;i<queries;++i) { (void)hm.find(static_cast<Language::UInt64>(i%items), &out); }
        auto t1 = ::Foundation::Time::SteadyClock::Now();
        double s = ::Foundation::Time::SteadyClock::DeltaSeconds(t0, t1);
        double ops = (s>0.0) ? (static_cast<double>(queries)/s) : 0.0;
        return ops;
    }

    static inline double bench_allocator_ops(IAllocator* a, Language::USize iters) noexcept {
        ::Foundation::Time::SteadyClock::Init();
        auto t0 = ::Foundation::Time::SteadyClock::Now();
        for (Language::USize i=0;i<iters;++i) {
            auto r = a->Allocate(static_cast<Language::USize>(64), static_cast<Language::USize>(16));
            if (r.IsOk()) { a->Deallocate(r.OkValue(), static_cast<Language::USize>(64), static_cast<Language::USize>(16)); }
        }
        auto t1 = ::Foundation::Time::SteadyClock::Now();
        double s = ::Foundation::Time::SteadyClock::DeltaSeconds(t0, t1);
        double ops = (s>0.0) ? (static_cast<double>(iters*2)/s) : 0.0;
        return ops;
    }

    static inline double bench_stream_read_mb(Language::USize bytes) noexcept {
        Language::Byte buf[4096]{};
        auto dr = ::Foundation::IO::Path::CreateDirectory(Language::StringView("Build/Reports"));
        (void)dr;
        auto wr = ::Foundation::IO::File::Open(Language::StringView("Build/Reports/MainlineB.temp"), ::Foundation::IO::FileOpenMode::Write, ::Foundation::IO::FileShareMode::Read);
        if (!wr.IsOk()) return 0.0; auto fhw = wr.OkValue();
        Language::USize written = 0;
        while (written < bytes) {
            Language::USize chunk = static_cast<Language::USize>((bytes - written) > sizeof(buf) ? sizeof(buf) : (bytes - written));
            (void)::Foundation::IO::File::Write(fhw, buf, chunk);
            written += chunk;
        }
        (void)::Foundation::IO::File::Close(fhw);
        auto rr = ::Foundation::IO::File::Open(Language::StringView("Build/Reports/MainlineB.temp"), ::Foundation::IO::FileOpenMode::Read, ::Foundation::IO::FileShareMode::Read);
        if (!rr.IsOk()) return 0.0; auto fhr = rr.OkValue();
        ::Foundation::IO::FileInputStream fis(fhr);
        ::Foundation::Time::SteadyClock::Init();
        auto t0 = ::Foundation::Time::SteadyClock::Now();
        Language::USize read = 0;
        while (read < bytes) {
            Language::USize chunk = static_cast<Language::USize>((bytes - read) > sizeof(buf) ? sizeof(buf) : (bytes - read));
            auto rs = fis.Read(Language::Span<Language::Byte>(buf, chunk));
            if (!rs.IsOk()) break;
            read += rs.OkValue();
        }
        auto t1 = ::Foundation::Time::SteadyClock::Now();
        (void)::Foundation::IO::File::Close(fhr);
        double s = ::Foundation::Time::SteadyClock::DeltaSeconds(t0, t1);
        double mbps = (s>0.0) ? ((static_cast<double>(read) / s) / (1024.0*1024.0)) : 0.0;
        return mbps;
    }

    export bool Run() noexcept {
        auto ar = CreateDefaultAllocator(); if (!ar.IsOk()) return false; auto* alloc = reinterpret_cast<IAllocator*>(ar.OkValue());
        double sv_ops = bench_smallvector_inserts(alloc, static_cast<Language::USize>(10'000'000));
        double hm_ops = bench_hashmap_reads(alloc, static_cast<Language::USize>(1'000'000), static_cast<Language::USize>(5'000'000));
        double alloc_ops = bench_allocator_ops(alloc, static_cast<Language::USize>(5'000'000));
        double io_mbps = bench_stream_read_mb(static_cast<Language::USize>(64*1024*1024));

        auto dr = ::Foundation::IO::Path::CreateDirectory(Language::StringView("Build")); (void)dr;
        dr = ::Foundation::IO::Path::CreateDirectory(Language::StringView("Build/Reports")); (void)dr;

        {
            Language::StaticString<256> s;
            s.append(Language::StringView("{\"smallvector_ops_s\":"));
            Language::UInt32 sv = static_cast<Language::UInt32>(sv_ops);
            Language::Char8 sbuf[16]{}; Language::USize sn=0; if (sv==0){ sbuf[sn++]=static_cast<Language::Char8>('0'); } else { Language::UInt32 x=sv; Language::Char8 tmp[16]{}; Language::USize t=0; while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; } while (t){ sbuf[sn++]=tmp[--t]; } }
            for (Language::USize i=0;i<sn;++i) s.push_back(sbuf[i]);
            s.append(Language::StringView(",\"hashmap_ops_s\":"));
            Language::UInt32 hv = static_cast<Language::UInt32>(hm_ops);
            Language::Char8 hbuf[16]{}; Language::USize hn=0; if (hv==0){ hbuf[hn++]=static_cast<Language::Char8>('0'); } else { Language::UInt32 x=hv; Language::Char8 tmp[16]{}; Language::USize t=0; while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; } while (t){ hbuf[hn++]=tmp[--t]; } }
            for (Language::USize i=0;i<hn;++i) s.push_back(hbuf[i]);
            s.append(Language::StringView(",\"allocator_ops_s\":"));
            Language::UInt32 av = static_cast<Language::UInt32>(alloc_ops);
            Language::Char8 abuf[16]{}; Language::USize an=0; if (av==0){ abuf[an++]=static_cast<Language::Char8>('0'); } else { Language::UInt32 x=av; Language::Char8 tmp[16]{}; Language::USize t=0; while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; } while (t){ abuf[an++]=tmp[--t]; } }
            for (Language::USize i=0;i<an;++i) s.push_back(abuf[i]);
            s.append(Language::StringView(",\"io_read_mbps\":"));
            Language::UInt32 iv = static_cast<Language::UInt32>(io_mbps);
            Language::Char8 ibuf[16]{}; Language::USize in=0; if (iv==0){ ibuf[in++]=static_cast<Language::Char8>('0'); } else { Language::UInt32 x=iv; Language::Char8 tmp[16]{}; Language::USize t=0; while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; } while (t){ ibuf[in++]=tmp[--t]; } }
            for (Language::USize i=0;i<in;++i) s.push_back(ibuf[i]);
            s.append(Language::StringView("}"));
            auto frj = ::Foundation::IO::File::Open(Language::StringView("Build/Reports/Mainline-B-DataMemory.json"), ::Foundation::IO::FileOpenMode::Write, ::Foundation::IO::FileShareMode::Read);
            if (frj.IsOk()) { auto fj = frj.OkValue(); (void)::Foundation::IO::File::Write(fj, reinterpret_cast<const Language::Byte*>(s.data()), static_cast<Language::USize>(s.size())); (void)::Foundation::IO::File::Close(fj); }
        }
        {
            Language::StaticString<256> s;
            s.append(Language::StringView("smallvector_ops_s,hashmap_ops_s,allocator_ops_s,io_read_mbps\n"));
            Language::UInt32 sv = static_cast<Language::UInt32>(sv_ops);
            Language::Char8 buf1[16]{}; Language::USize n1=0; if (sv==0){ buf1[n1++]=static_cast<Language::Char8>('0'); } else { Language::UInt32 x=sv; Language::Char8 tmp[16]{}; Language::USize t=0; while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; } while (t){ buf1[n1++]=tmp[--t]; } }
            for (Language::USize i=0;i<n1;++i) s.push_back(buf1[i]); s.append(Language::StringView(","));
            Language::UInt32 hv = static_cast<Language::UInt32>(hm_ops);
            Language::Char8 buf2[16]{}; Language::USize n2=0; if (hv==0){ buf2[n2++]=static_cast<Language::Char8>('0'); } else { Language::UInt32 x=hv; Language::Char8 tmp[16]{}; Language::USize t=0; while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; } while (t){ buf2[n2++]=tmp[--t]; } }
            for (Language::USize i=0;i<n2;++i) s.push_back(buf2[i]); s.append(Language::StringView(","));
            Language::UInt32 av = static_cast<Language::UInt32>(alloc_ops);
            Language::Char8 buf3[16]{}; Language::USize n3=0; if (av==0){ buf3[n3++]=static_cast<Language::Char8>('0'); } else { Language::UInt32 x=av; Language::Char8 tmp[16]{}; Language::USize t=0; while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; } while (t){ buf3[n3++]=tmp[--t]; } }
            for (Language::USize i=0;i<n3;++i) s.push_back(buf3[i]); s.append(Language::StringView(","));
            Language::UInt32 iv = static_cast<Language::UInt32>(io_mbps);
            Language::Char8 buf4[16]{}; Language::USize n4=0; if (iv==0){ buf4[n4++]=static_cast<Language::Char8>('0'); } else { Language::UInt32 x=iv; Language::Char8 tmp[16]{}; Language::USize t=0; while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; } while (t){ buf4[n4++]=tmp[--t]; } }
            for (Language::USize i=0;i<n4;++i) s.push_back(buf4[i]); s.append(Language::StringView("\n"));
            auto frc = ::Foundation::IO::File::Open(Language::StringView("Build/Reports/Mainline-B-DataMemory.csv"), ::Foundation::IO::FileOpenMode::Write, ::Foundation::IO::FileShareMode::Read);
            if (frc.IsOk()) { auto fc = frc.OkValue(); (void)::Foundation::IO::File::Write(fc, reinterpret_cast<const Language::Byte*>(s.data()), static_cast<Language::USize>(s.size())); (void)::Foundation::IO::File::Close(fc); }
        }
        return true;
    }
}