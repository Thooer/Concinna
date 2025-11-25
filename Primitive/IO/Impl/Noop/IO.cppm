module;
module Platform;

import Prm;
import :FileSystem;

namespace Platform {
    

    FileHandle File::Stdout() noexcept { return FileHandle{nullptr}; }
    FileHandle File::Stderr() noexcept { return FileHandle{nullptr}; }

    Expect<FileHandle> File::Open(StringView, FileOpenMode, FileShareMode) noexcept {
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

    bool Path::Exists(StringView) noexcept { return false; }
    bool Path::IsDirectory(StringView) noexcept { return false; }
    Status Path::CreateDirectory(StringView) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
    Status Path::RemoveFile(StringView) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }

    Status Directory::Walk(StringView, DirVisitor, void*) noexcept { return Err(StatusDomain::System(), StatusCode::Unsupported); }
}