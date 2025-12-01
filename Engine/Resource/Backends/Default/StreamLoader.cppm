module;
export module Engine.Resource:StreamLoader;

import Lang;
import Foundation.IO;

export namespace Engine::Resource {
    [[nodiscard]] inline Status ReadExactly(::Foundation::IO::IInputStream& in, Span<Byte> dst) noexcept {
        USize total = 0;
        while (total < dst.size()) {
            Span<Byte> chunk(dst.data() + total, dst.size() - total);
            auto r = in.Read(chunk);
            if (!r.IsOk()) return r.ErrStatus();
            auto got = r.OkValue();
            if (got == 0) return Err(StatusDomain::Platform(), StatusCode::Failed, "ReadExactly: EOF");
            total += got;
        }
        return Ok(StatusDomain::Platform());
    }

    [[nodiscard]] inline Status WriteExactly(::Foundation::IO::IOutputStream& out, Span<const Byte> src) noexcept {
        USize total = 0;
        while (total < src.size()) {
            Span<const Byte> chunk(src.data() + total, src.size() - total);
            auto r = out.Write(chunk);
            if (!r.IsOk()) return r.ErrStatus();
            auto sent = r.OkValue();
            if (sent == 0) return Err(StatusDomain::Platform(), StatusCode::Failed, "WriteExactly: short write");
            total += sent;
        }
        return Ok(StatusDomain::Platform());
    }
}