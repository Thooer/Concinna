module;
export module Prm.IO:FileSystem;
import Element;
import Flow;

export namespace Prm {
    export using FileHandle = void*;

    export enum class FileOpenMode : UInt32 { Read, Write, ReadWrite, Append, Create, CreateNew, Truncate };
    export enum class FileShareMode : UInt32 { None, Read, Write, ReadWrite, Delete };
    export enum class SeekOrigin : UInt32 { Begin, Current, End };
    export enum class MapAccess : UInt32 { Read, Write, ReadWrite };

    export struct Mapping { void* address{nullptr}; USize length{0}; void* nativeMappingHandle{nullptr}; };

    export class File {
    public:
        static Expect<FileHandle> Open(Span<const Char8, DynamicExtent> path, FileOpenMode mode, FileShareMode share) noexcept;
        static Status Close(FileHandle h) noexcept;
        static Expect<USize> Read(FileHandle h, Span<Byte, DynamicExtent> buffer) noexcept;
        static Expect<USize> Write(FileHandle h, Span<const Byte, DynamicExtent> buffer) noexcept;
        static Expect<USize> Write(FileHandle h, const Byte* data, USize len) noexcept;
        static Expect<UInt64> Size(FileHandle h) noexcept;
        static Expect<UInt64> Seek(FileHandle h, Int64 offset, SeekOrigin origin) noexcept;
        static Expect<Mapping> Map(FileHandle h, UInt64 offset, USize size, MapAccess access) noexcept;
        static Status Unmap(const Mapping& mapping) noexcept;
        static Status FlushMapped(FileHandle h, void* address, USize size) noexcept;
        static FileHandle Stdout() noexcept;
        static FileHandle Stderr() noexcept;

        static Status ReadAsync(FileHandle h, Span<Byte, DynamicExtent> buffer, UInt64 offset, void* user) noexcept;
        static Status WriteAsync(FileHandle h, Span<const Byte, DynamicExtent> data, UInt64 offset, void* user) noexcept;
        static Status CancelAsync(FileHandle h) noexcept;
        static Expect<bool> PollAsync(FileHandle h, UInt32 timeoutMs) noexcept;
    };

    export class Path {
    public:
        static bool   Exists(Span<const Char8, DynamicExtent> path) noexcept;
        static bool   IsDirectory(Span<const Char8, DynamicExtent> path) noexcept;
        static Status CreateDirectory(Span<const Char8, DynamicExtent> path) noexcept;
        static Status RemoveFile(Span<const Char8, DynamicExtent> path) noexcept;
    };
}
