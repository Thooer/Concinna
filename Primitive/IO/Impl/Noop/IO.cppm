module;
module Prm.IO;

import :FileSystem;

namespace Prm {
    

    FileHandle File::Stdout() noexcept { return nullptr; }
    FileHandle File::Stderr() noexcept { return nullptr; }

    Expect<FileHandle> File::Open(Span<const Char8, DynamicExtent>, FileOpenMode, FileShareMode) noexcept {
        return Expect<FileHandle>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Status File::Close(FileHandle) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }

    Expect<USize> File::Read(FileHandle, Span<Byte, DynamicExtent>) noexcept {
        return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Expect<USize> File::Write(FileHandle, Span<const Byte, DynamicExtent>) noexcept {
        return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Expect<USize> File::Write(FileHandle, const Byte*, USize) noexcept {
        return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }

    Expect<UInt64> File::Size(FileHandle) noexcept {
        return Expect<UInt64>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Expect<UInt64> File::Seek(FileHandle, Int64, SeekOrigin) noexcept {
        return Expect<UInt64>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }

    Expect<Mapping> File::Map(FileHandle, UInt64, USize, MapAccess) noexcept {
        return Expect<Mapping>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Status File::Unmap(const Mapping&) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status File::FlushMapped(FileHandle, void*, USize) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }

    bool Path::Exists(Span<const Char8, DynamicExtent>) noexcept { return false; }
    bool Path::IsDirectory(Span<const Char8, DynamicExtent>) noexcept { return false; }
    Status Path::CreateDirectory(Span<const Char8, DynamicExtent>) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status Path::RemoveFile(Span<const Char8, DynamicExtent>) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }

    

    Status File::ReadAsync(FileHandle, Span<Byte, DynamicExtent>, UInt64, AsyncRequest&) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status File::WriteAsync(FileHandle, Span<const Byte, DynamicExtent>, UInt64, AsyncRequest&) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status File::CancelAsync(const AsyncRequest&) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Expect<bool> File::CheckAsync(const AsyncRequest&, bool, USize&) noexcept { return Expect<bool>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); }
}
