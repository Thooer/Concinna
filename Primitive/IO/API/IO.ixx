// Platform.FileSystem — 文件系统原语：路径、文件句柄、读写、目录遍历
module;
export module Platform:FileSystem;

import Prm;

export namespace Platform {

    struct FileHandleTag;
    export using FileHandle = StrongAlias<void*, FileHandleTag, AliasHash, AliasEquals, AliasCompare>;

    enum class FileOpenMode : UInt32 { Read = 0, Write, ReadWrite, Append };
    enum class FileShareMode : UInt32 { None = 0, Read, Write, ReadWrite };
    enum class SeekOrigin : UInt32 { Begin = 0, Current, End };

    enum class MapAccess : UInt32 { Read = 0, Write, ReadWrite };

    struct Mapping {
        void* address{nullptr};
        USize  length{0};
        void* nativeMappingHandle{nullptr};
    };

    class Path {
    public:
        [[nodiscard]] static bool   Exists(StringView path) noexcept;
        [[nodiscard]] static bool   IsDirectory(StringView path) noexcept;
        [[nodiscard]] static Status CreateDirectory(StringView path) noexcept;
        [[nodiscard]] static Status RemoveFile(StringView path) noexcept;
    };

    export class File {
    public:
        [[nodiscard]] static Expect<FileHandle> Open(StringView path, FileOpenMode mode, FileShareMode share) noexcept;
        [[nodiscard]] static Status Close(FileHandle h) noexcept;

        [[nodiscard]] static Expect<USize> Read(FileHandle h, Span<Byte, DynamicExtent> buffer) noexcept;
        [[nodiscard]] static Expect<USize> Write(FileHandle h, Span<const Byte, DynamicExtent> buffer) noexcept;
        [[nodiscard]] static Expect<USize> Write(FileHandle h, const Byte* data, USize len) noexcept;

        [[nodiscard]] static Expect<UInt64> Size(FileHandle h) noexcept;
        [[nodiscard]] static Expect<UInt64> Seek(FileHandle h, Int64 offset, SeekOrigin origin) noexcept;

        [[nodiscard]] static Expect<Mapping> Map(FileHandle h, UInt64 offset, USize size, MapAccess access) noexcept;
        [[nodiscard]] static Status Unmap(const Mapping& mapping) noexcept;
        [[nodiscard]] static Status FlushMapped(FileHandle h, void* address, USize size) noexcept;

        [[nodiscard]] static FileHandle Stdout() noexcept;
        [[nodiscard]] static FileHandle Stderr() noexcept;
    };

    struct DirectoryFindTag;
    export using DirectoryFindHandle = StrongAlias<void*, DirectoryFindTag, AliasHash, AliasEquals, AliasCompare>;

    export struct DirectoryEntry {
        const Char8* name;
        USize        length;
        bool         isDirectory;
    };

    export class Directory {
    public:
        [[nodiscard]] static Expect<DirectoryFindHandle> FindFirst(StringView pattern, DirectoryEntry& out) noexcept;
        [[nodiscard]] static bool  FindNext(DirectoryFindHandle h, DirectoryEntry& out) noexcept;
        [[nodiscard]] static Status FindClose(DirectoryFindHandle h) noexcept;
    };
}
