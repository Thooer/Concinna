module;
export module Engine.Resource:StreamLoader;

import Lang;
import Foundation.IO;

export namespace Engine::Resource {
    [[nodiscard]] inline Language::Status ReadExactly(::Foundation::IO::IInputStream& in, Language::Span<Language::Byte> dst) noexcept {
        Language::USize total = 0;
        while (total < dst.size()) {
            Language::Span<Language::Byte> chunk(dst.data() + total, dst.size() - total);
            auto r = in.Read(chunk);
            if (!r.IsOk()) return r.ErrStatus();
            auto got = r.OkValue();
            if (got == 0) return Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed, "ReadExactly: EOF");
            total += got;
        }
        return Language::Ok(Language::StatusDomain::Platform());
    }

    [[nodiscard]] inline Language::Status WriteExactly(::Foundation::IO::IOutputStream& out, Language::Span<const Language::Byte> src) noexcept {
        Language::USize total = 0;
        while (total < src.size()) {
            Language::Span<const Language::Byte> chunk(src.data() + total, src.size() - total);
            auto r = out.Write(chunk);
            if (!r.IsOk()) return r.ErrStatus();
            auto sent = r.OkValue();
            if (sent == 0) return Language::Err(Language::StatusDomain::Platform(), Language::StatusCode::Failed, "WriteExactly: short write");
            total += sent;
        }
        return Language::Ok(Language::StatusDomain::Platform());
    }
}