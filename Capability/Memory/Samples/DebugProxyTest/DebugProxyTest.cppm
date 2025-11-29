import Lang;
import Memory;

extern "C" int main() {
    using namespace Memory;
    SystemMemoryResource system{};
    DebugMemoryResource debug{ &system };
    Allocator a{ &debug };

    auto r = a.Alloc(16, Alignment::Default);
    if (!r.IsOk()) return -1;
    auto blk = r.Value();
    auto* bytes = static_cast<Byte*>(blk.ptr);
    bytes[16] = 0xFF;
    a.Free(blk, Alignment::Default);
    return 0;
}