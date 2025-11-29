// 动态库加载与符号查询（C ABI 封装的类型安全接口）
module;
export module Prm.DynamicLibrary;

import Lang.Element;
import Lang.Flow;
import Lang.Text;
import Lang.Paradigm;
extern "C" __declspec(dllimport) void* LoadLibraryA(const char* path);
extern "C" __declspec(dllimport) int   FreeLibrary(void* h);
extern "C" __declspec(dllimport) void* GetProcAddress(void* h, const char* name);

export namespace Prm {
    

    using LibraryHandle = void*;
    using ProcAddress   = void*;

    class DynamicLibrary {
    public:
        [[nodiscard]] static Expect<LibraryHandle> Load(StringView path) noexcept {
            const char* p = reinterpret_cast<const char*>(path.data());
            void* h = LoadLibraryA(p);
            if (!h) return Expect<LibraryHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
            return Expect<LibraryHandle>::Ok(h);
        }

        [[nodiscard]] static Status Unload(LibraryHandle h) noexcept {
            int ok = FreeLibrary(h);
            return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
        }

        [[nodiscard]] static Expect<ProcAddress> Find(LibraryHandle h, StringView name) noexcept {
            const char* n = reinterpret_cast<const char*>(name.data());
            void* p = GetProcAddress(h, n);
            if (!p) return Expect<ProcAddress>::Err(Err(StatusDomain::System(), StatusCode::Failed));
            return Expect<ProcAddress>::Ok(p);
        }
    };
}
