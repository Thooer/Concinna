export module Cap.Concurrency:Job;

import Language;

export namespace Cap {
    enum class QoS : UInt8 { Normal = 0, High = 1 };

    struct Job {
        using Fn = void(*)(void*) noexcept;
        Fn invoke{nullptr};
        void* arg{nullptr};
        UInt8 qos{0};
        void Run() noexcept;
    };

    template<typename F>
    struct JobOf {
        F fn;
        static void Thunk(void* p) noexcept {
            auto* self = static_cast<JobOf*>(p);
            self->fn();
        }
        Job AsJob() noexcept { Job j{ &Thunk, this }; j.qos = static_cast<UInt8>(QoS::Normal); return j; }
    };
}
