import Language;
import Memory;
import Platform;

extern "C" int main() {
    using namespace Memory;
    FrameAllocatorResource res{ 1u << 20 };
    Allocator a{ &res };
    auto b1 = a.Alloc(256, Alignment::Default);
    if (!b1.IsOk()) return 1;
    auto b2 = a.Alloc(512, Alignment::Default);
    if (!b2.IsOk()) return 2;
    res.Reset();
    auto b3 = a.Alloc(1024, Alignment::Default);
    if (!b3.IsOk()) return 3;
    a.Free(b3.Value(), Alignment::Default);
    return 0;
}