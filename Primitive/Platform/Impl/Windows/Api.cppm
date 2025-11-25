// Windows backend: GetPlatformAPI() 提供函数表入口
module Platform;

import Prm;
import :Api;
import :FileSystem;
import :Threading;
import :Memory;
import :System;
import :Time;
import :Window;
import :Socket;
import :Input;
import :Audio;
import :Debug;
import :Clipboard;
import :File;


using namespace Platform;
 

// --- 动态库：Win32 入口声明 ---
extern "C" __declspec(dllimport) void* LoadLibraryA(const char* path);
extern "C" __declspec(dllimport) int   FreeLibrary(void* h);
extern "C" __declspec(dllimport) void* GetProcAddress(void* h, const char* name);

// 细小桥接：从 StringView 转为 C 字符串指针（假设视图持有以 NUL 结尾的缓冲区）
static Expect<void*> LibraryLoadSV(StringView path) noexcept {
    const char* cpath = reinterpret_cast<const char*>(path.data());
    void* h = LoadLibraryA(cpath);
    return h ? Expect<void*>::Ok(h)
             : Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::NotFound));
}
static Status LibraryFree(void* h) noexcept {
    const int ok = FreeLibrary(h);
    return ok ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
}
static Expect<void*> LibraryGetProc(void* h, StringView name) noexcept {
    const char* cname = reinterpret_cast<const char*>(name.data());
    void* p = GetProcAddress(h, cname);
    return p ? Expect<void*>::Ok(p)
             : Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::NotFound));
}

// --- 平台函数表 ---
static const PlatformAPI g_platformAPI = []() noexcept {
    PlatformAPI api{};

    // File
    api.file.Open        = &File::Open;
    api.file.Close       = &File::Close;
    api.file.Read        = &File::Read;
    api.file.WriteSpan   = static_cast<Expect<USize>(*)(FileHandle, Span<const Byte, DynamicExtent>) noexcept>(&File::Write);
    api.file.Write       = static_cast<Expect<USize>(*)(FileHandle, const Byte*, USize) noexcept>(&File::Write);
    api.file.Size        = &File::Size;
    api.file.Seek        = &File::Seek;
    api.file.Map         = &File::Map;
    api.file.Unmap       = &File::Unmap;
    api.file.FlushMapped = &File::FlushMapped;
    api.file.Stdout      = &File::Stdout;
    api.file.Stderr      = &File::Stderr;
    api.file.PathExists          = &Path::Exists;
    api.file.PathIsDirectory     = &Path::IsDirectory;
    api.file.PathCreateDirectory = &Path::CreateDirectory;
    api.file.PathRemoveFile      = &Path::RemoveFile;

    // Threading（自由函数 + 不透明句柄）
    api.threading.ThreadCreate  = &ThreadCreate;
    api.threading.ThreadJoin    = &ThreadJoin;
    api.threading.ThreadYield   = &ThreadYield;
    api.threading.ThreadSleepMs = &ThreadSleepMs;

    api.threading.MutexCreate   = &MutexCreate;
    api.threading.MutexDestroy  = &MutexDestroy;
    api.threading.MutexLock     = &MutexLock;
    api.threading.MutexUnlock   = &MutexUnlock;
    api.threading.MutexTryLock  = &MutexTryLock;

    api.threading.SemaphoreCreate  = &SemaphoreCreate;
    api.threading.SemaphoreDestroy = &SemaphoreDestroy;
    api.threading.SemaphoreAcquire = &SemaphoreAcquire;
    api.threading.SemaphoreRelease = &SemaphoreRelease;

    api.threading.EventCreate   = &EventCreate;
    api.threading.EventDestroy  = &EventDestroy;
    api.threading.EventWait     = &EventWait;
    api.threading.EventSignal   = &EventSignal;
    api.threading.EventReset    = &EventReset;

    // Memory
    api.memory.VirtualReserve            = &Platform::Memory::VirtualMemory::Reserve;
    api.memory.VirtualCommit             = &Platform::Memory::VirtualMemory::Commit;
    api.memory.VirtualDecommit           = &Platform::Memory::VirtualMemory::Decommit;
    api.memory.VirtualRelease            = &Platform::Memory::VirtualMemory::Release;
    api.memory.VirtualPageSize           = &Platform::Memory::VirtualMemory::PageSize;
    api.memory.VirtualAllocationGranularity = &Platform::Memory::VirtualMemory::AllocationGranularity;
    api.memory.LargePageSize             = &Platform::Memory::VirtualMemory::LargePageSize;
    api.memory.Protect                   = &Platform::Memory::VirtualMemory::Protect;
    api.memory.VirtualReserveEx          = &Platform::Memory::VirtualMemory::ReserveEx;

    api.memory.HeapCreate           = &Platform::Memory::Heap::Create;
    api.memory.HeapDestroy          = &Platform::Memory::Heap::Destroy;
    api.memory.HeapGetProcessDefault= &Platform::Memory::Heap::GetProcessDefault;
    api.memory.HeapAlloc            = &Platform::Memory::Heap::Alloc;
    api.memory.HeapFree             = &Platform::Memory::Heap::Free;
    api.memory.HeapMaximumAlignment = &Platform::Memory::Heap::MaximumAlignment;

    // System
    api.system.Kernel           = &SystemInfo::Kernel;
    api.system.Cpu              = &SystemInfo::Cpu;
    api.system.Memory           = &SystemInfo::Memory;
    api.system.GetNumaInfo      = &SystemInfo::GetNumaInfo;
    api.system.QuerySystemMemoryInfo = &SystemInfo::QuerySystemMemoryInfo;
    api.system.HostName         = &SystemInfo::HostName;
    api.system.UserName         = &SystemInfo::UserName;
    api.system.EnumerateDisplays= &Device::EnumerateDisplays;
    api.system.CaptureStackTrace= &CaptureStackTrace;

    // Time
    api.time.Now            = &Time::Now;
    api.time.SleepMs        = &Time::SleepMs;

    api.time.WallNow    = &WallClock::Now;
    api.time.WallUtcNow = &WallClock::UtcNow;

    // Window
    api.window.Create          = &Window::Create;
    api.window.Destroy         = &Window::Destroy;
    api.window.Show            = &Window::Show;
    api.window.Hide            = &Window::Hide;
    api.window.SetTitle        = &Window::SetTitle;
    api.window.Resize          = &Window::Resize;
    api.window.Native          = &Window::Native;
    api.window.ProcessOneMessage = &Window::ProcessOneMessage;
    api.window.SetCursorMode   = &Window::SetCursorMode;

    // Library
    api.library.Load   = &LibraryLoadSV;
    api.library.Free   = &LibraryFree;
    api.library.GetProc= &LibraryGetProc;

    api.asyncFile.ReadAsync  = &File::ReadAsync;
    api.asyncFile.WriteAsync = &File::WriteAsync;
    api.asyncFile.CancelAsync= &File::CancelAsync;
    api.asyncFile.PollAsync  = &File::PollAsync;

    api.input.Initialize     = &Input::Initialize;
    api.input.Shutdown       = &Input::Shutdown;
    api.input.GetMouseState  = &Mouse::GetState;
    api.input.GetKeyboardState = &Keyboard::GetState;

    api.audio.EnumerateDevices = &Audio::EnumerateDevices;

    api.debug.Break  = &Debug::Break;
    api.debug.Output = &Debug::Output;

    api.clipboard.SetText = &Clipboard::SetText;
    api.clipboard.GetText = &Clipboard::GetText;

    api.filePath.Normalize = &FilePath::Normalize;
    api.filePath.Join      = &FilePath::Join;
    api.filePath.Basename  = &FilePath::Basename;

    api.socket.Create       = &Socket::Create;
    api.socket.Close        = &Socket::Close;
    api.socket.Bind         = &Socket::Bind;
    api.socket.Listen       = &Socket::Listen;
    api.socket.Accept       = &Socket::Accept;
    api.socket.Connect      = &Socket::Connect;
    api.socket.Send         = &Socket::Send;
    api.socket.Recv         = &Socket::Recv;
    api.socket.Shutdown     = &Socket::Shutdown;
    api.socket.SetNonBlocking= &Socket::SetNonBlocking;
    api.socket.SetReuseAddr = &Socket::SetReuseAddr;
    api.socket.SetNoDelay   = &Socket::SetNoDelay;
    api.socket.MakeIPv4     = &Socket::MakeIPv4;
    api.socket.MakeIPv6     = &Socket::MakeIPv6;

    return api;
}();

// 稳定入口：供引擎/插件通过C ABI获取函数表
extern "C" const PlatformAPI* GetPlatformAPI() noexcept {
    return &g_platformAPI;
}
