module;
export module Concurrency.IOCPFileDemo;
import Language;
import Platform;
import Concurrency;

extern "C" __declspec(dllimport) int ReadFile(void* hFile, void* lpBuffer, unsigned long nNumberOfBytesToRead, unsigned long* lpNumberOfBytesRead, void* lpOverlapped);
extern "C" __declspec(dllimport) int GetOverlappedResult(void* hFile, void* lpOverlapped, unsigned long* lpNumberOfBytesTransferred, int bWait);

struct Ov { unsigned long long Internal; unsigned long long InternalHigh; unsigned long Offset; unsigned long OffsetHigh; void* Pointer; void* hEvent; };

static void AsyncRead(void* p) noexcept {
    auto* path = static_cast<const char*>(p);
    auto fh = Platform::File::Open(StringView{path}, Platform::FileOpenMode::Read, Platform::FileShareMode::Read);
    if (!fh.IsOk()) return;
    auto h = fh.Value();
    (void)Concurrency::gDriverApi->RegisterIocpHandle(h.Get());
    unsigned long toRead = 64 * 1024;
    auto heap = Platform::Memory::Heap::GetProcessDefault();
    auto rb = Platform::Memory::Heap::AllocRaw(heap, toRead);
    if (!rb.IsOk()) { (void)Platform::File::Close(h); return; }
    void* buf = rb.Value();
    auto ro = Platform::Memory::Heap::AllocRaw(heap, sizeof(Ov));
    if (!ro.IsOk()) { (void)Platform::Memory::Heap::FreeRaw(heap, buf); (void)Platform::File::Close(h); return; }
    auto* ov = static_cast<Ov*>(ro.Value());
    ov->Internal = 0; ov->InternalHigh = 0; ov->Offset = 0; ov->OffsetHigh = 0; ov->Pointer = nullptr; ov->hEvent = nullptr;
    unsigned long bytes = 0;
    int ok = ReadFile(h.Get(), buf, toRead, &bytes, ov);
    auto reg = +[](Concurrency::Fiber* f, void* ctx) noexcept {
        auto* o = static_cast<Ov*>(ctx);
        (void)Concurrency::gDriverApi->AwaitIocp(o, f);
    };
    Concurrency::Suspend(reg, ov);
    unsigned long got = 0;
    (void)GetOverlappedResult(h.Get(), ov, &got, 0);
    (void)Platform::Memory::Heap::FreeRaw(heap, ov);
    (void)Platform::Memory::Heap::FreeRaw(heap, buf);
    (void)Platform::File::Close(h);
}

export int main() {
    auto& sch = Concurrency::Scheduler::Instance();
    (void)sch.Start(0);
    Concurrency::Counter c{};
    c.Reset(1);
    const char* path = "README.md";
    Concurrency::Job j{}; j.invoke = &AsyncRead; j.arg = const_cast<char*>(path);
    (void)Concurrency::RunWithCounter(j, c);
    Concurrency::WaitForCounter(c);
    (void)sch.Stop();
    return 0;
}