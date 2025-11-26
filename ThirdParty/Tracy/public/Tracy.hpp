#pragma once

#ifdef TRACY_ENABLE

#include <cstdint>
#include <chrono>

// Simplified Tracy implementation for Nova engine testing
namespace tracy 
{
    // Basic zone instrumentation
    class [[nodiscard]] Zone 
    {
    public:
        Zone(const char* name) noexcept 
        {
            // Simplified - just record the name for now
            (void)name;
        }

        ~Zone() noexcept 
        {
            // Simplified - cleanup
        }
    };

    // Frame mark
    static inline void FrameMark() noexcept {}
    
    // Frame start
    static inline void FrameMarkStart(const char* name) noexcept 
    {
        (void)name;
    }
    
    // Frame end
    static inline void FrameMarkEnd(const char* name) noexcept 
    {
        (void)name;
    }

    // Memory allocations (simplified)
    static inline void SecureZeroMemory(void* ptr, size_t size) noexcept 
    {
        // Simplified zeroing
        volatile char* p = static_cast<volatile char*>(ptr);
        while (size--) *p++ = 0;
    }

    // Message logging
    static inline void Message(const char* text, int color = 0) noexcept 
    {
        (void)text;
        (void)color;
    }

    // Plot data
    // Plot data - multiple overloads for different types
    static inline void Plot(const char* name, double value) noexcept 
    {
        (void)name;
        (void)value;
    }

    static inline void Plot(const char* name, float value) noexcept 
    {
        (void)name;
        (void)value;
    }

    static inline void Plot(const char* name, int64_t value) noexcept 
    {
        (void)name;
        (void)value;
    }

    static inline void Plot(const char* name, uint64_t value) noexcept 
    {
        (void)name;
        (void)value;
    }

    static inline void Plot(const char* name, int32_t value) noexcept 
    {
        (void)name;
        (void)value;
    }

    static inline void Plot(const char* name, uint32_t value) noexcept 
    {
        (void)name;
        (void)value;
    }

    // Custom name for current thread
    static inline void SetThreadName(const char* name) noexcept 
    {
        (void)name;
    }

    // Alloc/free hooks (simplified)
    static inline void OnAllocMem(void* ptr, size_t size) noexcept 
    {
        (void)ptr;
        (void)size;
    }

    static inline void OnFreeMem(void* ptr) noexcept 
    {
        (void)ptr;
    }

    // Performance counter
    static inline void ConfigureExternalServer(const char* host, uint16_t port) noexcept 
    {
        (void)host;
        (void)port;
    }

    // Connection info
    enum class ConnectionState : int 
    {
        Connecting = 0,
        Connected = 1,
        Disconnected = 2
    };

    static inline ConnectionState GetConnectionState() noexcept 
    {
        return ConnectionState::Disconnected;
    }

    static inline void ProfilerFlushRequests() noexcept {}
}

// Macro definitions for easier use
#define TRACY_ZONE(name) tracy::Zone __tracy_zone(name)
#define TRACY_FRAME_MARK() tracy::FrameMark()
#define TRACY_FRAME_MARK_START(name) tracy::FrameMarkStart(name)
#define TRACY_FRAME_MARK_END(name) tracy::FrameMarkEnd(name)
#define TRACY_MESSAGE(text) tracy::Message(text)
#define TRACY_MESSAGE_COL(text, color) tracy::Message(text, color)
#define TRACY_PLOT(name, value) tracy::Plot(name, value)
#define TRACY_SET_THREAD_NAME(name) tracy::SetThreadName(name)

#else

// Empty implementations when Tracy is disabled
namespace tracy 
{
    class [[nodiscard]] Zone 
    {
    public:
        Zone(const char*) noexcept {}
        ~Zone() noexcept {}
    };
    
    static inline void FrameMark() noexcept {}
    static inline void FrameMarkStart(const char*) noexcept {}
    static inline void FrameMarkEnd(const char*) noexcept {}
    static inline void Message(const char*) noexcept {}
    static inline void Plot(const char*, double) noexcept {}
    static inline void SetThreadName(const char*) noexcept {}
    static inline void OnAllocMem(void*, size_t) noexcept {}
    static inline void OnFreeMem(void*) noexcept {}
    static inline void SecureZeroMemory(void*, size_t) noexcept {}
    static inline void ConfigureExternalServer(const char*, uint16_t) noexcept {}
    static inline void ProfilerFlushRequests() noexcept {}
}

#define TRACY_ZONE(name) tracy::Zone __tracy_zone__disabled(name)
#define TRACY_FRAME_MARK() tracy::FrameMark()
#define TRACY_FRAME_MARK_START(name) tracy::FrameMarkStart(name)
#define TRACY_FRAME_MARK_END(name) tracy::FrameMarkEnd(name)
#define TRACY_MESSAGE(text) tracy::Message(text)
#define TRACY_MESSAGE_COL(text, color) tracy::Message(text, color)
#define TRACY_PLOT(name, value) tracy::Plot(name, value)
#define TRACY_SET_THREAD_NAME(name) tracy::SetThreadName(name)

#endif

// Helper class for profiling with RAII
class ScopedTracyZone 
{
public:
    explicit ScopedTracyZone(const char* name) noexcept 
    {
        m_zone = new tracy::Zone(name);
    }

    ~ScopedTracyZone() noexcept 
    {
        delete m_zone;
    }

    // Non-copyable, non-movable
    ScopedTracyZone(const ScopedTracyZone&) = delete;
    ScopedTracyZone& operator=(const ScopedTracyZone&) = delete;
    ScopedTracyZone(ScopedTracyZone&&) = delete;
    ScopedTracyZone& operator=(ScopedTracyZone&&) = delete;

private:
    tracy::Zone* m_zone;
};