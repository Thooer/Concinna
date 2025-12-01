module;
export module Engine.Resource.VersioningBatchSmoke;

import Lang;
import Engine.Resource;
import Cap.Memory;
import Cap.Containers;
import Foundation.Time;
import Foundation.IO;

namespace Nova::Samples::ResourceVerBatch {
  using Engine::Resource::ResourceManager;
  export bool Run() noexcept {
    auto ar = Foundation::Memory::CreateDefaultAllocator();
    if (!ar.IsOk()) return false; auto* alloc = reinterpret_cast<Foundation::Memory::IAllocator*>(ar.OkValue());
    ResourceManager mgr(alloc);
    Foundation::Containers::Vector<StringView> keys(alloc);
    for (UInt32 i=0;i<1000;++i) {
      StaticString<64> k; k.append(StringView("mesh_"));
      Char8 buf[16]{}; USize n=0; if (i==0){ buf[n++]=static_cast<Char8>('0'); }
      else { UInt32 x=i%200; Char8 tmp[16]{}; USize t=0; while (x>0){ tmp[t++]=static_cast<Char8>('0'+(x%10)); x/=10; } while (t){ buf[n++]=tmp[--t]; } }
      for (USize j=0;j<n;++j) k.push_back(buf[j]);
      keys.push_back(StringView(k.data(), k.size()));
    }
    auto t0 = Foundation::Time::SteadyClock::Now();
    auto r = mgr.LoadBatch(Span<const StringView>(keys.data(), keys.size()));
    auto t1 = Foundation::Time::SteadyClock::Now();
    UInt32 okc = r.IsOk() ? r.OkValue() : 0u;
    double ms = Foundation::Time::SteadyClock::DeltaSeconds(t0, t1) * 1000.0;
    auto hr = Foundation::IO::Path::CreateDirectory(StringView("Build"));
    if (!hr.Ok()) return false;
    hr = Foundation::IO::Path::CreateDirectory(StringView("Build/Reports"));
    if (!hr.Ok()) return false;
    {
      StaticString<256> s;
      s.append(StringView("assets_loaded,avg_latency_ms\n"));
      Char8 buf[32]{}; USize n=0; {
        UInt32 x = okc; Char8 tmp[16]{}; USize t=0; if (x==0){ tmp[t++]=static_cast<Char8>('0'); }
        while (x>0){ tmp[t++]=static_cast<Char8>('0'+(x%10)); x/=10; }
        while (t){ buf[n++]=tmp[--t]; }
      }
      for (USize j=0;j<n;++j) s.push_back(buf[j]);
      s.append(StringView(","));
      UInt32 msInt = static_cast<UInt32>(ms);
      Char8 b2[32]{}; USize n2=0; {
        UInt32 x = msInt; Char8 tmp[16]{}; USize t=0; if (x==0){ tmp[t++]=static_cast<Char8>('0'); }
        while (x>0){ tmp[t++]=static_cast<Char8>('0'+(x%10)); x/=10; }
        while (t){ b2[n2++]=tmp[--t]; }
      }
      for (USize j=0;j<n2;++j) s.push_back(b2[j]);
      s.append(StringView("\n"));
      auto frc = Foundation::IO::File::Open(StringView("Build/Reports/Resource.Manager.csv"), Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::Read);
      if (frc.IsOk()) {
        auto fc = frc.OkValue();
        (void)Foundation::IO::File::Write(fc, reinterpret_cast<const Byte*>(s.data()), static_cast<USize>(s.size()));
        (void)Foundation::IO::File::Close(fc);
      }
    }
    {
      StaticString<256> s;
      s.append(StringView("{\"assets_loaded\":"));
      Char8 buf[32]{}; USize n=0; {
        UInt32 x = okc; Char8 tmp[16]{}; USize t=0; if (x==0){ tmp[t++]=static_cast<Char8>('0'); }
        while (x>0){ tmp[t++]=static_cast<Char8>('0'+(x%10)); x/=10; }
        while (t){ buf[n++]=tmp[--t]; }
      }
      for (USize j=0;j<n;++j) s.push_back(buf[j]);
      s.append(StringView(",\"avg_latency_ms\":"));
      UInt32 msInt = static_cast<UInt32>(ms);
      Char8 b2[32]{}; USize n2=0; {
        UInt32 x = msInt; Char8 tmp[16]{}; USize t=0; if (x==0){ tmp[t++]=static_cast<Char8>('0'); }
        while (x>0){ tmp[t++]=static_cast<Char8>('0'+(x%10)); x/=10; }
        while (t){ b2[n2++]=tmp[--t]; }
      }
      for (USize j=0;j<n2;++j) s.push_back(b2[j]);
      s.append(StringView("}"));
      auto frj = Foundation::IO::File::Open(StringView("Build/Reports/Resource.Manager.json"), Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::Read);
      if (frj.IsOk()) {
        auto fj = frj.OkValue();
        (void)Foundation::IO::File::Write(fj, reinterpret_cast<const Byte*>(s.data()), static_cast<USize>(s.size()));
        (void)Foundation::IO::File::Close(fj);
      }
    }
    return true;
  }
}