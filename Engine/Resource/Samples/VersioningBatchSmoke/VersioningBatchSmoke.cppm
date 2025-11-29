module;
export module Engine.Resource.VersioningBatchSmoke;

import Lang;
import Engine.Resource;
import Foundation.Memory;
import Foundation.Containers;
import Foundation.Time;
import Foundation.IO;

namespace Nova::Samples::ResourceVerBatch {
  using Engine::Resource::ResourceManager;
  export bool Run() noexcept {
    auto ar = Foundation::Memory::CreateDefaultAllocator();
    if (!ar.IsOk()) return false; auto* alloc = reinterpret_cast<Foundation::Memory::IAllocator*>(ar.OkValue());
    ResourceManager mgr(alloc);
    Foundation::Containers::Vector<Language::StringView> keys(alloc);
    for (Language::UInt32 i=0;i<1000;++i) {
      Language::StaticString<64> k; k.append(Language::StringView("mesh_"));
      Language::Char8 buf[16]{}; Language::USize n=0; if (i==0){ buf[n++]=static_cast<Language::Char8>('0'); }
      else { Language::UInt32 x=i%200; Language::Char8 tmp[16]{}; Language::USize t=0; while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; } while (t){ buf[n++]=tmp[--t]; } }
      for (Language::USize j=0;j<n;++j) k.push_back(buf[j]);
      keys.push_back(Language::StringView(k.data(), k.size()));
    }
    auto t0 = Foundation::Time::SteadyClock::Now();
    auto r = mgr.LoadBatch(Language::Span<const Language::StringView>(keys.data(), keys.size()));
    auto t1 = Foundation::Time::SteadyClock::Now();
    Language::UInt32 okc = r.IsOk() ? r.OkValue() : 0u;
    double ms = Foundation::Time::SteadyClock::DeltaSeconds(t0, t1) * 1000.0;
    auto hr = Foundation::IO::Path::CreateDirectory(Language::StringView("Build"));
    if (!hr.Ok()) return false;
    hr = Foundation::IO::Path::CreateDirectory(Language::StringView("Build/Reports"));
    if (!hr.Ok()) return false;
    {
      Language::StaticString<256> s;
      s.append(Language::StringView("assets_loaded,avg_latency_ms\n"));
      Language::Char8 buf[32]{}; Language::USize n=0; {
        Language::UInt32 x = okc; Language::Char8 tmp[16]{}; Language::USize t=0; if (x==0){ tmp[t++]=static_cast<Language::Char8>('0'); }
        while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; }
        while (t){ buf[n++]=tmp[--t]; }
      }
      for (Language::USize j=0;j<n;++j) s.push_back(buf[j]);
      s.append(Language::StringView(","));
      Language::UInt32 msInt = static_cast<Language::UInt32>(ms);
      Language::Char8 b2[32]{}; Language::USize n2=0; {
        Language::UInt32 x = msInt; Language::Char8 tmp[16]{}; Language::USize t=0; if (x==0){ tmp[t++]=static_cast<Language::Char8>('0'); }
        while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; }
        while (t){ b2[n2++]=tmp[--t]; }
      }
      for (Language::USize j=0;j<n2;++j) s.push_back(b2[j]);
      s.append(Language::StringView("\n"));
      auto frc = Foundation::IO::File::Open(Language::StringView("Build/Reports/Resource.Manager.csv"), Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::Read);
      if (frc.IsOk()) {
        auto fc = frc.OkValue();
        (void)Foundation::IO::File::Write(fc, reinterpret_cast<const Language::Byte*>(s.data()), static_cast<Language::USize>(s.size()));
        (void)Foundation::IO::File::Close(fc);
      }
    }
    {
      Language::StaticString<256> s;
      s.append(Language::StringView("{\"assets_loaded\":"));
      Language::Char8 buf[32]{}; Language::USize n=0; {
        Language::UInt32 x = okc; Language::Char8 tmp[16]{}; Language::USize t=0; if (x==0){ tmp[t++]=static_cast<Language::Char8>('0'); }
        while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; }
        while (t){ buf[n++]=tmp[--t]; }
      }
      for (Language::USize j=0;j<n;++j) s.push_back(buf[j]);
      s.append(Language::StringView(",\"avg_latency_ms\":"));
      Language::UInt32 msInt = static_cast<Language::UInt32>(ms);
      Language::Char8 b2[32]{}; Language::USize n2=0; {
        Language::UInt32 x = msInt; Language::Char8 tmp[16]{}; Language::USize t=0; if (x==0){ tmp[t++]=static_cast<Language::Char8>('0'); }
        while (x>0){ tmp[t++]=static_cast<Language::Char8>('0'+(x%10)); x/=10; }
        while (t){ b2[n2++]=tmp[--t]; }
      }
      for (Language::USize j=0;j<n2;++j) s.push_back(b2[j]);
      s.append(Language::StringView("}"));
      auto frj = Foundation::IO::File::Open(Language::StringView("Build/Reports/Resource.Manager.json"), Foundation::IO::FileOpenMode::Write, Foundation::IO::FileShareMode::Read);
      if (frj.IsOk()) {
        auto fj = frj.OkValue();
        (void)Foundation::IO::File::Write(fj, reinterpret_cast<const Language::Byte*>(s.data()), static_cast<Language::USize>(s.size()));
        (void)Foundation::IO::File::Close(fj);
      }
    }
    return true;
  }
}