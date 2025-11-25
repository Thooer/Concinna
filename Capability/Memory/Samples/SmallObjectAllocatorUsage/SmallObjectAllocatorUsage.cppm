import Language;
import Memory;

extern "C" int main() {
    using namespace Memory;
    SystemMemoryResource system{};
    SmallObjectAllocatorResource soa(1 << 20, &system);
    Allocator a{ &soa };

    auto r1 = a.Alloc(13, Alignment::Default);
    if (!r1.IsOk()) return -1;
    auto r2 = a.Alloc(40, Alignment::Default);
    if (!r2.IsOk()) return -2;

    a.Free(r1.Value(), Alignment::Default);
    a.Free(r2.Value(), Alignment::Default);
    return 0;
}