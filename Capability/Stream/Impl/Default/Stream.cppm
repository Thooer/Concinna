module Cap.Stream;

import Cap.Stream;
import Lang;
import Prm.IO;

using namespace Cap;

Expect<USize> BinaryReader::ReadBytes(Span<Byte, DynamicExtent> dst) noexcept {
    if (!m_src) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
    auto r = m_src->ReadBytes(dst);
    if (!r.IsOk()) return Expect<USize>::Err(r.Error());
    return Expect<USize>::Ok(r.Value().bytes);
}

Expect<FileReader> FileReader::Open(Span<const Char8, DynamicExtent> path) noexcept {
    auto r = Prm::File::Open(path, Prm::FileOpenMode::Read, Prm::FileShareMode::Read);
    if (!r.IsOk()) return Expect<FileReader>::Err(r.Error());
    return Expect<FileReader>::Ok(FileReader{ r.Value() });
}

Expect<StreamReadResult> FileReader::ReadBytes(Span<Byte, DynamicExtent> dst) noexcept {
    if (!m_h) return Expect<StreamReadResult>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
    auto r = Prm::File::Read(m_h, dst);
    if (!r.IsOk()) return Expect<StreamReadResult>::Err(r.Error());
    StreamReadResult rr{}; rr.bytes = r.Value(); rr.eof = (rr.bytes == 0);
    return Expect<StreamReadResult>::Ok(rr);
}

Expect<UInt64> FileReader::Seek(Int64 offset, UInt32 whence) noexcept {
    if (!m_h) return Expect<UInt64>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
    return Prm::File::Seek(m_h, offset, static_cast<Prm::SeekOrigin>(whence));
}

Expect<FileWriter> FileWriter::Open(Span<const Char8, DynamicExtent> path, bool append) noexcept {
    auto r = Prm::File::Open(path, append ? Prm::FileOpenMode::Append : Prm::FileOpenMode::Write, Prm::FileShareMode::ReadWrite);
    if (!r.IsOk()) return Expect<FileWriter>::Err(r.Error());
    return Expect<FileWriter>::Ok(FileWriter{ r.Value() });
}

Expect<StreamWriteResult> FileWriter::WriteBytes(Span<const Byte, DynamicExtent> src) noexcept {
    if (!m_h) return Expect<StreamWriteResult>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
    auto r = Prm::File::Write(m_h, src);
    if (!r.IsOk()) return Expect<StreamWriteResult>::Err(r.Error());
    StreamWriteResult wr{}; wr.bytes = r.Value();
    return Expect<StreamWriteResult>::Ok(wr);
}

Expect<UInt64> FileWriter::Seek(Int64 offset, UInt32 whence) noexcept {
    if (!m_h) return Expect<UInt64>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
    return Prm::File::Seek(m_h, offset, static_cast<Prm::SeekOrigin>(whence));
}

Expect<MemoryMappedFile> MemoryMappedFile::Map(Prm::FileHandle h, UInt64 offset, USize size, Prm::MapAccess access) noexcept {
    auto r = Prm::File::Map(h, offset, size, access);
    if (!r.IsOk()) return Expect<MemoryMappedFile>::Err(r.Error());
    MemoryMappedFile mm{}; mm.m_map = r.Value();
    return Expect<MemoryMappedFile>::Ok(mm);
}

void MemoryMappedFile::Unmap() noexcept {
    if (m_map.address) { (void)Prm::File::Unmap(m_map); m_map.address = nullptr; m_map.length = 0; m_map.nativeMappingHandle = nullptr; }
}
