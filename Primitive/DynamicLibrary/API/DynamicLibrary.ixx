// 动态库加载与符号查询（C ABI 封装的类型安全接口）
module;
export module Prm.DynamicLibrary;

import Prm.Element;
import Prm.Flow;
import Prm.Text;
import Prm.Paradigm;
import Prm.Platform; // 使用 PlatformAPI vtable 进行库操作

export namespace Prm {
    

    using LibraryHandle = void*;
    using ProcAddress   = void*;

    class DynamicLibrary {
    public:
        [[nodiscard]] static Expect<LibraryHandle> Load(StringView path) noexcept {
            const auto api = GetPlatformAPI();
            if (!api || !api->library.Load) {
                return Expect<LibraryHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
            }
            return api->library.Load(path);
        }

        [[nodiscard]] static Status Unload(LibraryHandle h) noexcept {
            const auto api = GetPlatformAPI();
            if (!api || !api->library.Free) {
                return Err(StatusDomain::System(), StatusCode::Failed);
            }
            return api->library.Free(h);
        }

        [[nodiscard]] static Expect<ProcAddress> Find(LibraryHandle h, StringView name) noexcept {
            const auto api = GetPlatformAPI();
            if (!api || !api->library.GetProc) {
                return Expect<ProcAddress>::Err(Err(StatusDomain::System(), StatusCode::Failed));
            }
            return api->library.GetProc(h, name);
        }
    };
}
