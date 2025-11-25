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

static const PlatformAPI g_platformAPI_noop = []() noexcept {
    PlatformAPI api{};

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

    api.asyncFile.ReadAsync  = &File::ReadAsync;
    api.asyncFile.WriteAsync = &File::WriteAsync;
    api.asyncFile.CancelAsync= &File::CancelAsync;
    api.asyncFile.PollAsync  = &File::PollAsync;

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

    api.system.Kernel           = &SystemInfo::Kernel;
    api.system.Cpu              = &SystemInfo::Cpu;
    api.system.Memory           = &SystemInfo::Memory;
    api.system.GetNumaInfo      = &SystemInfo::GetNumaInfo;
    api.system.QuerySystemMemoryInfo = &SystemInfo::QuerySystemMemoryInfo;
    api.system.HostName         = &SystemInfo::HostName;
    api.system.UserName         = &SystemInfo::UserName;
    api.system.EnumerateDisplays= &Device::EnumerateDisplays;
    api.system.CaptureStackTrace= &CaptureStackTrace;

    api.time.Now            = &Time::Now;
    api.time.SleepMs        = &Time::SleepMs;
    api.time.WallNow        = &WallClock::Now;
    api.time.WallUtcNow     = &WallClock::UtcNow;

    api.window.Create          = &Window::Create;
    api.window.Destroy         = &Window::Destroy;
    api.window.Show            = &Window::Show;
    api.window.Hide            = &Window::Hide;
    api.window.SetTitle        = &Window::SetTitle;
    api.window.Resize          = &Window::Resize;
    api.window.Native          = &Window::Native;
    api.window.ProcessOneMessage = &Window::ProcessOneMessage;
    api.window.SetCursorMode   = &Window::SetCursorMode;

    api.library.Load   = [](StringView) noexcept -> Expect<void*> { return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); };
    api.library.Free   = [](void*) noexcept -> Status { return Err(StatusDomain::System(), StatusCode::Unsupported); };
    api.library.GetProc= [](void*, StringView) noexcept -> Expect<void*> { return Expect<void*>::Err(Err(StatusDomain::System(), StatusCode::Unsupported)); };

    api.socket.Create        = &Socket::Create;
    api.socket.Close         = &Socket::Close;
    api.socket.Bind          = &Socket::Bind;
    api.socket.Listen        = &Socket::Listen;
    api.socket.Accept        = &Socket::Accept;
    api.socket.Connect       = &Socket::Connect;
    api.socket.Send          = &Socket::Send;
    api.socket.Recv          = &Socket::Recv;
    api.socket.Shutdown      = &Socket::Shutdown;
    api.socket.SetNonBlocking= &Socket::SetNonBlocking;
    api.socket.SetReuseAddr  = &Socket::SetReuseAddr;
    api.socket.SetNoDelay    = &Socket::SetNoDelay;
    api.socket.MakeIPv4      = &Socket::MakeIPv4;
    api.socket.MakeIPv6      = &Socket::MakeIPv6;

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

    return api;
}();

extern "C" const PlatformAPI* GetPlatformAPI() noexcept { return &g_platformAPI_noop; }
