import Language;
import Memory;
import Platform;
import <thread>;

extern "C" int main() {
    using namespace Memory;
    ThreadMemory::Init();
    PoolAllocatorResource res{ 64, 1u << 20 };
    Allocator a{ &res };
    auto b1 = a.Alloc(32, Alignment::Default);
    if (!b1.IsOk()) return 1;
    auto b2 = a.Alloc(48, Alignment::Default);
    if (!b2.IsOk()) return 2;
    void* p1 = b1.Value().ptr;
    void* p2 = b2.Value().ptr;
    std::thread t([&] {
        ThreadMemory::Init();
        a.Free(MemoryBlock{ p1, 32 }, Alignment::Default);
        a.Free(MemoryBlock{ p2, 48 }, Alignment::Default);
        ThreadMemory::Shutdown();
    });
    t.join();
    auto b3 = a.Alloc(64, Alignment::Default);
    if (!b3.IsOk()) return 3;
    a.Free(b3.Value(), Alignment::Default);
    ThreadMemory::Shutdown();
    return 0;
}