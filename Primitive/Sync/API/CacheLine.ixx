export module Prm.Sync:CacheLine;
import Element;

export namespace Prm {
    template<typename T>
    struct alignas(CACHELINE_SIZE) CacheLineAligned { T value; };
    struct CacheLinePad { char pad[CACHELINE_SIZE]; };
}

