#include "Tracy.hpp"

#ifdef TRACY_ENABLE

#include <cstdlib>
#include <cstdint>

namespace tracy 
{
    // Simplified allocation implementation
    void* Alloc(size_t size) 
    {
        void* ptr = malloc(size);
        OnAllocMem(ptr, size);
        return ptr;
    }

    void Free(void* ptr) 
    {
        if (ptr) 
        {
            OnFreeMem(ptr);
            free(ptr);
        }
    }

    // Simplified callstack capture (always returns empty for now)
    void* CaptureCallstack(int depth, const char* context, uint32_t type) 
    {
        (void)depth;
        (void)context;
        (void)type;
        return nullptr;  // Simplified - no actual callstack capture
    }
}

#endif