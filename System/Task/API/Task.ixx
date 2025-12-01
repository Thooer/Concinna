export module Sys.Task;
import Lang;

export namespace Sys {
    export struct TaskHandle { USize id{0}; };
    export TaskHandle CreateTask() noexcept;
    export bool AddDependency(TaskHandle, TaskHandle) noexcept;
    export bool Dispatch(TaskHandle) noexcept;
}

namespace Sys {
    TaskHandle CreateTask() noexcept { return TaskHandle{0}; }
    bool AddDependency(TaskHandle, TaskHandle) noexcept { return true; }
    bool Dispatch(TaskHandle) noexcept { return true; }
}
