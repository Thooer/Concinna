// Windows 后端：实现 FileSystem 的 Stdout/Stderr 与 Write
import Prm.IO;
import Flow;
import Element;

namespace Prm {
    
    // Win32 基础 API（避免包含 <windows.h>）
    extern "C" __declspec(dllimport) void* GetStdHandle(unsigned long nStdHandle);
    extern "C" __declspec(dllimport) int   WriteFile(void* hFile, const void* lpBuffer, unsigned long nNumberOfBytesToWrite, unsigned long* lpNumberOfBytesWritten, void* lpOverlapped);
    extern "C" __declspec(dllimport) void* CreateFileA(const char* lpFileName, unsigned long dwDesiredAccess, unsigned long dwShareMode, void* lpSecurityAttributes, unsigned long dwCreationDisposition, unsigned long dwFlagsAndAttributes, void* hTemplateFile);
    extern "C" __declspec(dllimport) int   CloseHandle(void* hObject);
    extern "C" __declspec(dllimport) int   ReadFile(void* hFile, void* lpBuffer, unsigned long nNumberOfBytesToRead, unsigned long* lpNumberOfBytesRead, void* lpOverlapped);
    extern "C" __declspec(dllimport) int   GetOverlappedResult(void* hFile, void* lpOverlapped, unsigned long* lpNumberOfBytesTransferred, int bWait);
    extern "C" __declspec(dllimport) int   CancelIoEx(void* hFile, void* lpOverlapped);
    extern "C" __declspec(dllimport) unsigned long GetLastError();
    extern "C" __declspec(dllimport) int   GetFileSizeEx(void* hFile, Int64* lpFileSize);
    extern "C" __declspec(dllimport) int   SetFilePointerEx(void* hFile, Int64 liDistanceToMove, Int64* lpNewFilePointer, unsigned long dwMoveMethod);
    extern "C" __declspec(dllimport) unsigned long GetFileAttributesA(const char* lpFileName);
    extern "C" __declspec(dllimport) int   CreateDirectoryA(const char* lpPathName, void* lpSecurityAttributes);
    extern "C" __declspec(dllimport) int   DeleteFileA(const char* lpFileName);
    extern "C" __declspec(dllimport) void* CreateFileMappingA(void* hFile, void* lpFileMappingAttributes, unsigned long flProtect, unsigned long dwMaximumSizeHigh, unsigned long dwMaximumSizeLow, const char* lpName);
    extern "C" __declspec(dllimport) void* MapViewOfFile(void* hFileMappingObject, unsigned long dwDesiredAccess, unsigned long dwFileOffsetHigh, unsigned long dwFileOffsetLow, UInt64 dwNumberOfBytesToMap);
    extern "C" __declspec(dllimport) int   UnmapViewOfFile(const void* lpBaseAddress);
    extern "C" __declspec(dllimport) int   FlushViewOfFile(const void* lpBaseAddress, unsigned long dwNumberOfBytesToFlush);

    // 常量：标准句柄（DWORD）
    static constexpr unsigned long kSTD_OUTPUT_HANDLE = static_cast<unsigned long>(-11); // (DWORD)-11
    static constexpr unsigned long kSTD_ERROR_HANDLE  = static_cast<unsigned long>(-12); // (DWORD)-12
    static constexpr unsigned long kGENERIC_READ   = 0x80000000ul;
    static constexpr unsigned long kGENERIC_WRITE  = 0x40000000ul;
    static constexpr unsigned long kFILE_APPEND_DATA = 0x00000004ul;
    static constexpr unsigned long kFILE_SHARE_READ  = 0x00000001ul;
    static constexpr unsigned long kFILE_SHARE_WRITE = 0x00000002ul;
    static constexpr unsigned long kOPEN_EXISTING    = 0x00000003ul;
    static constexpr unsigned long kCREATE_ALWAYS    = 0x00000002ul;
    static constexpr unsigned long kOPEN_ALWAYS      = 0x00000004ul;
    static constexpr unsigned long kFILE_ATTRIBUTE_NORMAL = 0x00000080ul;
    static constexpr unsigned long kFILE_ATTRIBUTE_DIRECTORY = 0x00000010ul;
    static constexpr unsigned long kINVALID_FILE_ATTRIBUTES = 0xFFFFFFFFul;
    static constexpr unsigned long kFILE_BEGIN   = 0u;
    static constexpr unsigned long kFILE_CURRENT = 1u;
    static constexpr unsigned long kFILE_END     = 2u;
    static void* const kINVALID_HANDLE_VALUE = (void*)-1;
    static constexpr unsigned long kPAGE_READONLY  = 0x00000002ul;
    static constexpr unsigned long kPAGE_READWRITE = 0x00000004ul;
    static constexpr unsigned long kFILE_MAP_READ      = 0x00000001ul;
    static constexpr unsigned long kFILE_MAP_WRITE     = 0x00000002ul;
    static constexpr unsigned long kFILE_MAP_READ_WRITE= kFILE_MAP_READ | kFILE_MAP_WRITE;
    static constexpr unsigned long kERROR_IO_PENDING   = 997ul;
    extern "C" __declspec(dllimport) void* FindFirstFileA(const char* lpFileName, void* lpFindFileData);
    extern "C" __declspec(dllimport) int   FindNextFileA(void* hFindFile, void* lpFindFileData);
    extern "C" __declspec(dllimport) int   FindClose(void* hFindFile);

    // 标准输出句柄（类静态）
    FileHandle File::Stdout() noexcept { return GetStdHandle(kSTD_OUTPUT_HANDLE); }

    // 标准错误句柄（类静态）
    FileHandle File::Stderr() noexcept { return GetStdHandle(kSTD_ERROR_HANDLE); }

    // 写入（Span）
    Expect<USize> File::Write(FileHandle h, Span<const Byte, DynamicExtent> buffer) noexcept {
        if (!h) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        unsigned long written = 0;
        const unsigned long toWrite = static_cast<unsigned long>(buffer.size());
        const int ok = WriteFile(h, buffer.data(), toWrite, &written, nullptr);
        if (!ok) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
        }
        return Expect<USize>::Ok(static_cast<USize>(written));
    }

    // 写入（指针 + 长度）
    Expect<USize> File::Write(FileHandle h, const Byte* data, USize len) noexcept {
        if (!h) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        unsigned long written = 0;
        const unsigned long toWrite = static_cast<unsigned long>(len);
        const int ok = WriteFile(h, data, toWrite, &written, nullptr);
        if (!ok) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
        }
        return Expect<USize>::Ok(static_cast<USize>(written));
    }

    // 自由函数包装
    FileHandle Stdout() noexcept { return File::Stdout(); }
    FileHandle Stderr() noexcept { return File::Stderr(); }
    Expect<USize> Write(FileHandle h, const Byte* data, USize len) noexcept { return File::Write(h, data, len); }

    // 路径相关
    bool Path::Exists(Span<const Char8, DynamicExtent> path) noexcept {
        const char* p = reinterpret_cast<const char*>(path.data());
        const unsigned long attrs = GetFileAttributesA(p);
        return attrs != kINVALID_FILE_ATTRIBUTES;
    }
    bool Path::IsDirectory(Span<const Char8, DynamicExtent> path) noexcept {
        const char* p = reinterpret_cast<const char*>(path.data());
        const unsigned long attrs = GetFileAttributesA(p);
        return (attrs != kINVALID_FILE_ATTRIBUTES) && ((attrs & kFILE_ATTRIBUTE_DIRECTORY) != 0);
    }
    Status Path::CreateDirectory(Span<const Char8, DynamicExtent> path) noexcept {
        const char* p = reinterpret_cast<const char*>(path.data());
        const int ok = CreateDirectoryA(p, nullptr);
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }
    Status Path::RemoveFile(Span<const Char8, DynamicExtent> path) noexcept {
        const char* p = reinterpret_cast<const char*>(path.data());
        const int ok = DeleteFileA(p);
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    // 文件句柄操作
    Expect<FileHandle> File::Open(Span<const Char8, DynamicExtent> path, FileOpenMode mode, FileShareMode share) noexcept {
        const char* p = reinterpret_cast<const char*>(path.data());
        unsigned long access = 0;
        unsigned long creation = kOPEN_EXISTING;
        switch (mode) {
            case FileOpenMode::Read: access = kGENERIC_READ; creation = kOPEN_EXISTING; break;
            case FileOpenMode::Write: access = kGENERIC_WRITE; creation = kCREATE_ALWAYS; break;
            case FileOpenMode::ReadWrite: access = kGENERIC_READ | kGENERIC_WRITE; creation = kOPEN_ALWAYS; break;
            case FileOpenMode::Append: access = kFILE_APPEND_DATA | kGENERIC_WRITE; creation = kOPEN_ALWAYS; break;
            default: access = kGENERIC_READ; creation = kOPEN_EXISTING; break;
        }
        unsigned long shareMode = 0;
        switch (share) {
            case FileShareMode::None: shareMode = 0; break;
            case FileShareMode::Read: shareMode = kFILE_SHARE_READ; break;
            case FileShareMode::Write: shareMode = kFILE_SHARE_WRITE; break;
            case FileShareMode::ReadWrite: shareMode = kFILE_SHARE_READ | kFILE_SHARE_WRITE; break;
            default: shareMode = 0; break;
        }

        void* h = CreateFileA(p, access, shareMode, nullptr, creation, kFILE_ATTRIBUTE_NORMAL, nullptr);
        if (!h || h == kINVALID_HANDLE_VALUE) {
            return Expect<FileHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        // 若为追加模式，将文件指针移动到末尾
        if (mode == FileOpenMode::Append) {
            Int64 newPos = 0;
            const int ok = SetFilePointerEx(h, 0, &newPos, kFILE_END);
            (void)ok;
        }
        return Expect<FileHandle>::Ok(h);
    }

    Status File::Close(FileHandle h) noexcept {
        if (!h || h == kINVALID_HANDLE_VALUE) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = CloseHandle(h);
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Expect<USize> File::Read(FileHandle h, Span<Byte, DynamicExtent> buffer) noexcept {
        if (!h || h == kINVALID_HANDLE_VALUE) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        unsigned long read = 0;
        const int ok = ReadFile(h, buffer.data(), static_cast<unsigned long>(buffer.size()), &read, nullptr);
        if (!ok) {
            return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        return Expect<USize>::Ok(static_cast<USize>(read));
    }

    Expect<UInt64> File::Size(FileHandle h) noexcept {
        if (!h || h == kINVALID_HANDLE_VALUE) {
            return Expect<UInt64>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        Int64 sz = 0;
        const int ok = GetFileSizeEx(h, &sz);
        if (!ok) { return Expect<UInt64>::Err(Err(StatusDomain::System(), StatusCode::Failed)); }
        return Expect<UInt64>::Ok(static_cast<UInt64>(sz));
    }

    Expect<UInt64> File::Seek(FileHandle h, Int64 offset, SeekOrigin origin) noexcept {
        if (!h || h == kINVALID_HANDLE_VALUE) {
            return Expect<UInt64>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        unsigned long o = kFILE_BEGIN;
        switch (origin) {
            case SeekOrigin::Begin:   o = kFILE_BEGIN;   break;
            case SeekOrigin::Current: o = kFILE_CURRENT; break;
            case SeekOrigin::End:     o = kFILE_END;     break;
            default: o = kFILE_BEGIN; break;
        }
        Int64 newPos = 0;
        const int ok = SetFilePointerEx(h, static_cast<Int64>(offset), &newPos, o);
        if (!ok) { return Expect<UInt64>::Err(Err(StatusDomain::System(), StatusCode::Failed)); }
        return Expect<UInt64>::Ok(static_cast<UInt64>(newPos));
    }

    // 内存映射 I/O
    Expect<Mapping> File::Map(FileHandle h, UInt64 offset, USize size, MapAccess access) noexcept {
        if (!h || h == kINVALID_HANDLE_VALUE || size == 0) {
            return Expect<Mapping>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        const unsigned long prot = (access == MapAccess::Read) ? kPAGE_READONLY : kPAGE_READWRITE;
        void* mapping = CreateFileMappingA(h, nullptr, prot,
                                           static_cast<unsigned long>((static_cast<UInt64>(size) >> 32) & 0xffffffffu),
                                           static_cast<unsigned long>(static_cast<UInt64>(size) & 0xffffffffu),
                                           nullptr);
        if (!mapping) {
            return Expect<Mapping>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        const unsigned long desired = (access == MapAccess::Read) ? kFILE_MAP_READ : ((access == MapAccess::Write) ? kFILE_MAP_WRITE : kFILE_MAP_READ_WRITE);
        void* addr = MapViewOfFile(mapping, desired,
                                   static_cast<unsigned long>((offset >> 32) & 0xffffffffu),
                                   static_cast<unsigned long>(offset & 0xffffffffu),
                                   static_cast<UInt64>(size));
        if (!addr) {
            (void)CloseHandle(mapping);
            return Expect<Mapping>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        Mapping m{}; m.address = addr; m.length = size; m.nativeMappingHandle = mapping;
        return Expect<Mapping>::Ok(m);
    }

    Status File::Unmap(const Mapping& mapping) noexcept {
        if (!mapping.address) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok1 = UnmapViewOfFile(mapping.address);
        int ok2 = 1;
        if (mapping.nativeMappingHandle) {
            ok2 = CloseHandle(mapping.nativeMappingHandle);
        }
        return (ok1 && ok2) ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Status File::FlushMapped(FileHandle /*h*/, void* address, USize size) noexcept {
        if (!address) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        const int ok = FlushViewOfFile(address, static_cast<unsigned long>(size));
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    struct OVERLAPPED {
        UIntPtr Internal;
        UIntPtr InternalHigh;
        UInt32  Offset;
        UInt32  OffsetHigh;
        void*   hEvent;
    };

    Status File::ReadAsync(FileHandle h, Span<Byte, DynamicExtent> buffer, UInt64 offset, AsyncRequest& out_req) noexcept {
        if (!h || h == kINVALID_HANDLE_VALUE || buffer.size() == 0) {
            return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        }
        if (!out_req.nativeOverlapped) {
            return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        }
        auto ov = static_cast<OVERLAPPED*>(out_req.nativeOverlapped);
        ov->Offset     = static_cast<UInt32>(offset & 0xffffffffu);
        ov->OffsetHigh = static_cast<UInt32>((offset >> 32) & 0xffffffffu);
        out_req.file = h;
        unsigned long ignored = 0;
        const int ok = ReadFile(h, buffer.data(), static_cast<unsigned long>(buffer.size()), &ignored, ov);
        if (!ok) {
            const unsigned long err = GetLastError();
            if (err != kERROR_IO_PENDING) {
                return Err(StatusDomain::System(), StatusCode::Failed);
            }
        }
        return Ok(StatusDomain::System());
    }

    Status File::WriteAsync(FileHandle h, Span<const Byte, DynamicExtent> data, UInt64 offset, AsyncRequest& out_req) noexcept {
        if (!h || h == kINVALID_HANDLE_VALUE || data.size() == 0) {
            return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        }
        if (!out_req.nativeOverlapped) {
            return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        }
        auto ov = static_cast<OVERLAPPED*>(out_req.nativeOverlapped);
        ov->Offset     = static_cast<UInt32>(offset & 0xffffffffu);
        ov->OffsetHigh = static_cast<UInt32>((offset >> 32) & 0xffffffffu);
        out_req.file = h;
        unsigned long ignored = 0;
        const int ok = WriteFile(h, data.data(), static_cast<unsigned long>(data.size()), &ignored, ov);
        if (!ok) {
            const unsigned long err = GetLastError();
            if (err != kERROR_IO_PENDING) {
                return Err(StatusDomain::System(), StatusCode::Failed);
            }
        }
        return Ok(StatusDomain::System());
    }

    Status File::CancelAsync(const AsyncRequest& req) noexcept {
        if (!req.file || !req.nativeOverlapped) {
            return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        }
        const int ok = CancelIoEx(req.file, req.nativeOverlapped);
        return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Expect<bool> File::CheckAsync(const AsyncRequest& req, bool wait, USize& out_bytes) noexcept {
        out_bytes = 0;
        if (!req.file || !req.nativeOverlapped) {
            return Expect<bool>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        }
        unsigned long transferred = 0;
        const int ok = GetOverlappedResult(req.file, req.nativeOverlapped, &transferred, wait ? 1 : 0);
        if (ok) {
            out_bytes = static_cast<USize>(transferred);
            return Expect<bool>::Ok(true);
        }
        const unsigned long err = GetLastError();
        if (err == kERROR_IO_PENDING && !wait) {
            return Expect<bool>::Ok(false);
        }
        return Expect<bool>::Err(Err(StatusDomain::System(), StatusCode::Failed));
    }

    
}
