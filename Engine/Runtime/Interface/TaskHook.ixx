module;
export module Engine.Runtime:TaskHook;

import Lang;
import :Types;

export namespace Engine {
    struct TaskHandle { USize id{0}; };
    struct TaskSystemHook {
        virtual ~TaskSystemHook() = default;
        virtual TaskHandle Submit(void(*fn)(void*), void* user) noexcept = 0;
        virtual bool IsReady(TaskHandle) const noexcept = 0;
        virtual void Fence() noexcept = 0;
        virtual bool EnqueuePriority(FunctionView<void() noexcept> fn, int level) noexcept = 0;
        virtual USize CreateJob(FunctionView<void() noexcept> fn) noexcept = 0;
        virtual bool AddEdge(USize src, USize dst) noexcept = 0;
        virtual USize MakeBarrier(USize prereqCount) noexcept = 0;
        virtual void StartGraph() noexcept = 0;
        virtual bool BuildStages() noexcept = 0;
        virtual bool RunStages() noexcept = 0;
    };
    inline TaskHandle MakeTaskHandle(USize id) noexcept { return TaskHandle{ id }; }
    struct NullTaskSystemHook final : TaskSystemHook {
        TaskHandle Submit(void(*)(void*), void*) noexcept override { return MakeTaskHandle(0); }
        bool IsReady(TaskHandle) const noexcept override { return true; }
        void Fence() noexcept override {}
        bool EnqueuePriority(FunctionView<void() noexcept>, int) noexcept override { return true; }
        USize CreateJob(FunctionView<void() noexcept>) noexcept override { return static_cast<USize>(0); }
        bool AddEdge(USize, USize) noexcept override { return true; }
        USize MakeBarrier(USize) noexcept override { return static_cast<USize>(0); }
        void StartGraph() noexcept override {}
        bool BuildStages() noexcept override { return true; }
        bool RunStages() noexcept override { return true; }
    };
    void SetTaskSystemHook(TaskSystemHook*) noexcept;
    TaskSystemHook* GetTaskSystemHook() noexcept;

    export inline TaskSystemHook* CreateDefaultTaskHook() noexcept {
        static NullTaskSystemHook hook{};
        return &hook;
    }
    export inline bool EnqueuePriority(FunctionView<void() noexcept> fn, int level) noexcept {
        auto* h = GetTaskSystemHook(); if (!h) return false; return h->EnqueuePriority(fn, level);
    }
    export template<typename F>
    inline bool EnqueuePriorityT(F& f, int level) noexcept {
        FunctionView<void() noexcept> view{f};
        return EnqueuePriority(view, level);
    }
    export inline USize CreateJob(FunctionView<void() noexcept> fn) noexcept {
        auto* h = GetTaskSystemHook(); if (!h) return static_cast<USize>(0); return h->CreateJob(fn);
    }
    export template<typename F>
    inline USize CreateJobT(F& f) noexcept {
        FunctionView<void() noexcept> view{f};
        return CreateJob(view);
    }
    export inline bool AddEdge(USize src, USize dst) noexcept {
        auto* h = GetTaskSystemHook(); if (!h) return false; return h->AddEdge(src, dst);
    }
    export inline USize MakeBarrier(USize prereqCount) noexcept {
        auto* h = GetTaskSystemHook(); if (!h) return static_cast<USize>(0); return h->MakeBarrier(prereqCount);
    }
    export inline void StartGraph() noexcept {
        auto* h = GetTaskSystemHook(); if (!h) return; h->StartGraph();
    }
    export inline bool BuildStages() noexcept {
        auto* h = GetTaskSystemHook(); if (!h) return false; return h->BuildStages();
    }
    export inline bool RunStages() noexcept {
        auto* h = GetTaskSystemHook(); if (!h) return false; return h->RunStages();
    }
}
