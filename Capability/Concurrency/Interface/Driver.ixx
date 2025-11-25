export module Concurrency:Driver;

import Language;
import Platform;
import :Fiber;

export namespace Concurrency {
    export struct IDriver {
        virtual bool Init() noexcept = 0;
        virtual void Shutdown() noexcept = 0;
        virtual void Poll() noexcept = 0;
        [[nodiscard]] virtual UInt32 NextTimeoutMs() noexcept = 0;
        virtual bool AddTimer(Fiber* fb, UInt32 delayMs) noexcept = 0;
        virtual bool AddTimeout(void(*cb)(void*) noexcept, void* ctx, UInt32 delayMs) noexcept = 0;
        virtual bool AddEvent(Platform::EventHandle h, Fiber* fb) noexcept = 0;
        virtual void WakeUp() noexcept = 0;
        virtual bool AttachIocp() noexcept = 0;
        virtual bool RegisterIocpHandle(void* h) noexcept = 0;
        virtual bool AwaitIocp(void* ov, Fiber* fb) noexcept = 0;
        [[nodiscard]] virtual UInt32 PendingCount() noexcept = 0;
        virtual ~IDriver() {}
    };
    struct Driver : IDriver {
        bool Init() noexcept;
        void Shutdown() noexcept;
        void Poll() noexcept;
        [[nodiscard]] UInt32 NextTimeoutMs() noexcept;
        bool AddTimer(Fiber* fb, UInt32 delayMs) noexcept;
        bool AddTimeout(void(*cb)(void*) noexcept, void* ctx, UInt32 delayMs) noexcept;
        bool AddEvent(Platform::EventHandle h, Fiber* fb) noexcept;
        void WakeUp() noexcept;
        bool AttachIocp() noexcept;
        bool RegisterIocpHandle(void* h) noexcept;
        bool AwaitIocp(void* ov, Fiber* fb) noexcept;
        [[nodiscard]] UInt32 PendingCount() noexcept;
    };
    export extern Driver gDriver;
    export extern IDriver* gDriverApi;
    export void SleepMs(UInt32 ms) noexcept;
}