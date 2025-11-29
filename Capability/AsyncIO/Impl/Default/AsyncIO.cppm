module Cap.AsyncIO;

import Cap.AsyncIO;
import Lang;
import Prm.IO;

using namespace Cap;

Expect<AsyncFile> AsyncFile::OpenRead(Span<const Char8, DynamicExtent> path) noexcept {
    auto r = Prm::File::Open(path, Prm::FileOpenMode::Read, Prm::FileShareMode::Read);
    if (!r.IsOk()) return Expect<AsyncFile>::Err(r.Error());
    return Expect<AsyncFile>::Ok(AsyncFile{ r.Value() });
}

Expect<AsyncFile> AsyncFile::OpenWrite(Span<const Char8, DynamicExtent> path, bool append) noexcept {
    auto r = Prm::File::Open(path, append ? Prm::FileOpenMode::Append : Prm::FileOpenMode::Write, Prm::FileShareMode::ReadWrite);
    if (!r.IsOk()) return Expect<AsyncFile>::Err(r.Error());
    return Expect<AsyncFile>::Ok(AsyncFile{ r.Value() });
}

Status AsyncFile::Read(Span<Byte, DynamicExtent> buffer, UInt64 offset, AsyncRequest& r) noexcept {
    if (!m_h) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
    return Prm::File::ReadAsync(m_h, buffer, offset, r.req);
}

Status AsyncFile::Write(Span<const Byte, DynamicExtent> data, UInt64 offset, AsyncRequest& r) noexcept {
    if (!m_h) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
    return Prm::File::WriteAsync(m_h, data, offset, r.req);
}

Status AsyncFile::Cancel(const AsyncRequest& r) noexcept {
    return Prm::File::CancelAsync(r.req);
}

Expect<bool> AsyncFile::Check(const AsyncRequest& r, bool wait, USize& outBytes) const noexcept {
    return Prm::File::CheckAsync(r.req, wait, outBytes);
}

void AsyncFile::Close() noexcept {
    if (m_h) { (void)Prm::File::Close(m_h); m_h = nullptr; }
}
