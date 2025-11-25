// Platform.Api — 纯C函数表（VTable）接口，将实现细节与链接方式解耦
module;
export module Prm.Platform:Api;

import Prm.Element;
import Prm.Flow;
import Prm.Paradigm;
import Prm.Text;

import Prm.IO;
import Prm.Threading;
import Prm.Ownership;
import Prm.System;
import Prm.Time;
import Prm.Window;
import Prm.Socket;
import Prm.Input;
import Prm.Audio;
import Prm.Debug;
import Prm.Clipboard;
import Prm.File;

export namespace Prm {
    // 文件系统 API（函数指针表）
    export struct FileAPI {
        Expect<FileHandle> (*Open)(StringView path, FileOpenMode mode, FileShareMode share) noexcept;
        Status (*Close)(FileHandle h) noexcept;

        Expect<USize> (*Read)(FileHandle h, Span<Byte, DynamicExtent> buffer) noexcept;
        Expect<USize> (*WriteSpan)(FileHandle h, Span<const Byte, DynamicExtent> buffer) noexcept;
        Expect<USize> (*Write)(FileHandle h, const Byte* data, USize len) noexcept;

        Expect<UInt64> (*Size)(FileHandle h) noexcept;
        Expect<UInt64> (*Seek)(FileHandle h, Int64 offset, SeekOrigin origin) noexcept;

        Expect<Mapping> (*Map)(FileHandle h, UInt64 offset, USize size, MapAccess access) noexcept;
        Status (*Unmap)(const Mapping& mapping) noexcept;
        Status (*FlushMapped)(FileHandle h, void* address, USize size) noexcept;

        FileHandle (*Stdout)() noexcept;
        FileHandle (*Stderr)() noexcept;

        bool   (*PathExists)(StringView path) noexcept;
        bool   (*PathIsDirectory)(StringView path) noexcept;
        Status (*PathCreateDirectory)(StringView path) noexcept;
        Status (*PathRemoveFile)(StringView path) noexcept;
    };

    export struct AsyncFileAPI {
        Status (*ReadAsync)(FileHandle h, Span<Byte, DynamicExtent> buffer, UInt64 offset, void* user) noexcept;
        Status (*WriteAsync)(FileHandle h, Span<const Byte, DynamicExtent> data, UInt64 offset, void* user) noexcept;
        Status (*CancelAsync)(FileHandle h) noexcept;
        Expect<bool> (*PollAsync)(FileHandle h, UInt32 timeoutMs) noexcept;
    };

    // 线程与同步 API（函数指针表）
    export struct ThreadingAPI {
        // Thread
        Expect<ThreadHandle> (*ThreadCreate)(ThreadFunc func, void* user) noexcept;
        Status (*ThreadJoin)(ThreadHandle h) noexcept;
        void   (*ThreadYield)() noexcept;
        void   (*ThreadSleepMs)(UInt32 ms) noexcept;

        // Mutex (opaque handle)
        Expect<MutexHandle> (*MutexCreate)() noexcept;
        Status (*MutexDestroy)(MutexHandle h) noexcept;
        Status (*MutexLock)(MutexHandle h) noexcept;
        Status (*MutexUnlock)(MutexHandle h) noexcept;
        Status (*MutexTryLock)(MutexHandle h) noexcept;

        // Semaphore (opaque handle)
        Expect<SemaphoreHandle> (*SemaphoreCreate)(UInt32 initial, UInt32 max) noexcept;
        Status (*SemaphoreDestroy)(SemaphoreHandle h) noexcept;
        Status (*SemaphoreAcquire)(SemaphoreHandle h, UInt32 timeoutMs) noexcept;
        Status (*SemaphoreRelease)(SemaphoreHandle h, UInt32 count) noexcept;

        // Event (opaque handle)
        Expect<EventHandle> (*EventCreate)(bool manualReset, bool initialState) noexcept;
        Status (*EventDestroy)(EventHandle h) noexcept;
        Status (*EventWait)(EventHandle h, UInt32 timeoutMs) noexcept;
        Status (*EventSignal)(EventHandle h) noexcept;
        Status (*EventReset)(EventHandle h) noexcept;
    };

    // 内存 API（函数指针表）
    export struct MemoryAPI {
        // Virtual memory primitives
        Expect<void*> (*VirtualReserve)(USize size) noexcept;
        Status (*VirtualCommit)(void* base, USize size, PageProtection protection) noexcept;
        Status (*VirtualDecommit)(void* base, USize size) noexcept;
        Status (*VirtualRelease)(void* base) noexcept;
        USize  (*VirtualPageSize)() noexcept;
        USize  (*VirtualAllocationGranularity)() noexcept;
        Expect<USize> (*LargePageSize)() noexcept;
        Status (*Protect)(void* base, USize size, PageProtection protection) noexcept;
        Expect<void*> (*VirtualReserveEx)(USize size, UInt32 numaNodeId, bool useLargePages) noexcept;

        // Process heap primitives
        Expect<HeapHandle> (*HeapCreate)() noexcept;
        Status (*HeapDestroy)(HeapHandle h) noexcept;
        HeapHandle (*HeapGetProcessDefault)() noexcept;
        Expect<void*> (*HeapAlloc)(HeapHandle h, USize size, USize alignment) noexcept;
        Status (*HeapFree)(HeapHandle h, void* p) noexcept;
        USize (*HeapMaximumAlignment)() noexcept;
    };

    // 系统信息 API（函数指针表）
    export struct SystemAPI {
        KernelInfo (*Kernel)() noexcept;
        CpuInfo    (*Cpu)() noexcept;
        MemoryInfo (*Memory)() noexcept;
        Expect<NumaInfo> (*GetNumaInfo)() noexcept;
        Expect<SystemMemoryInfo> (*QuerySystemMemoryInfo)() noexcept;
        Expect<USize> (*HostName)(Span<Char8, DynamicExtent> buffer) noexcept;
        Expect<USize> (*UserName)(Span<Char8, DynamicExtent> buffer) noexcept;
        Expect<USize> (*EnumerateDisplays)(Span<Device::DisplayInfo, DynamicExtent> out) noexcept;
        USize (*CaptureStackTrace)(Span<void*, DynamicExtent> buffer) noexcept;
    };

    // 时间 API（函数指针表）
    export struct TimeAPI {
        TimePoint (*Now)() noexcept;
        void (*SleepMs)(UInt32 milliseconds) noexcept;

        WallClock::Data (*WallNow)() noexcept;
        WallClock::Data (*WallUtcNow)() noexcept;
    };

    export struct InputAPI {
        Status (*Initialize)() noexcept;
        void   (*Shutdown)() noexcept;
        Expect<MouseState>    (*GetMouseState)() noexcept;
        Expect<KeyboardState> (*GetKeyboardState)() noexcept;
    };

    export struct AudioAPI {
        Expect<USize> (*EnumerateDevices)(Span<AudioDevice, DynamicExtent> out) noexcept;
    };

    export struct DebugAPI {
        void   (*Break)() noexcept;
        Status (*Output)(StringView text) noexcept;
    };

    export struct ClipboardAPI {
        Status (*SetText)(StringView text) noexcept;
        Expect<USize> (*GetText)(Span<Char8, DynamicExtent> buffer) noexcept;
    };

    export struct FilePathAPI {
        Expect<USize> (*Normalize)(StringView path, Span<Char8, DynamicExtent> out) noexcept;
        Expect<USize> (*Join)(StringView a, StringView b, Span<Char8, DynamicExtent> out) noexcept;
        Expect<USize> (*Basename)(StringView path, Span<Char8, DynamicExtent> out) noexcept;
    };

    // 窗口原语 API（函数指针表）
    export struct WindowAPI {
        Expect<WindowHandle> (*Create)(const WindowDesc& desc, WndProcCallback callback) noexcept;
        void   (*Destroy)(WindowHandle h) noexcept;
        Status (*Show)(WindowHandle h) noexcept;
        Status (*Hide)(WindowHandle h) noexcept;
        Status (*SetTitle)(WindowHandle h, StringView title) noexcept;
        Status (*Resize)(WindowHandle h, UInt32 w, UInt32 hgt) noexcept;
        Expect<void*> (*Native)(WindowHandle h) noexcept;
        bool   (*ProcessOneMessage)(WindowHandle h) noexcept;
        Status (*SetCursorMode)(WindowHandle h, CursorMode m) noexcept;
    };

    // 动态库 API（函数指针表）
    export struct LibraryAPI {
        using LibraryHandle = void*;
        using ProcAddress   = void*;

        Expect<LibraryHandle> (*Load)(StringView path) noexcept;
        Status (*Free)(LibraryHandle h) noexcept;
        Expect<ProcAddress> (*GetProc)(LibraryHandle h, StringView symbol) noexcept;
    };

    // 套接字 API（函数指针表）
    export struct SocketAPI {
        Expect<SocketHandle> (*Create)(AddressFamily af, SocketType type, Protocol proto) noexcept;
        Status (*Close)(SocketHandle h) noexcept;
        Status (*Bind)(SocketHandle h, const SocketAddress& addr) noexcept;
        Status (*Listen)(SocketHandle h, Int32 backlog) noexcept;
        Expect<SocketHandle> (*Accept)(SocketHandle h) noexcept;
        Status (*Connect)(SocketHandle h, const SocketAddress& addr) noexcept;
        Expect<USize> (*Send)(SocketHandle h, Span<const Byte, DynamicExtent> data) noexcept;
        Expect<USize> (*Recv)(SocketHandle h, Span<Byte, DynamicExtent> buffer) noexcept;
        Status (*Shutdown)(SocketHandle h, ShutdownMode how) noexcept;
        Status (*SetNonBlocking)(SocketHandle h, bool enable) noexcept;
        Status (*SetReuseAddr)(SocketHandle h, bool enable) noexcept;
        Status (*SetNoDelay)(SocketHandle h, bool enable) noexcept;
        Expect<SocketAddress> (*MakeIPv4)(StringView ip, UInt16 port) noexcept;
        Expect<SocketAddress> (*MakeIPv6)(StringView ip, UInt16 port) noexcept;
    };

    // 平台主函数表：聚合所有子系统
    export struct PlatformAPI {
        FileAPI      file;
        AsyncFileAPI asyncFile;
        ThreadingAPI threading;
        MemoryAPI    memory;
        SystemAPI    system;
        TimeAPI      time;
        WindowAPI    window;
        LibraryAPI   library;
        SocketAPI    socket;
        InputAPI     input;
        AudioAPI     audio;
        DebugAPI     debug;
        ClipboardAPI clipboard;
        FilePathAPI  filePath;
    };
    // C ABI 入口，由具体后端（如 Windows）提供
    export extern "C" const PlatformAPI* GetPlatformAPI() noexcept;
}
