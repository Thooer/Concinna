import Language;
import Memory;

extern "C" int main() {
    using namespace Memory;
    SystemMemoryResource res;
    Allocator a{ &res };
    auto b = a.Alloc(1024, Alignment::Default);
    if (!b.IsOk()) return 1;
    OwnedMemoryBlock omb{ a, b.Value(), Alignment::Default };
    auto r = res.Reallocate(omb.Release(), 2048, Alignment::Default);
    if (!r.IsOk()) return 2;
    a.Free(r.Value(), Alignment::Default);
    return 0;
}
