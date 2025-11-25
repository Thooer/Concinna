export module Concurrency:TaskGroup;

import Language;
import :Counter;
import :Scheduler;
import :Job;

export namespace Concurrency {
    struct TaskGroup {
        Counter c{};
        TaskGroup() noexcept;
        ~TaskGroup() noexcept;
        template<typename F>
        [[nodiscard]] Status Run(F&& f) noexcept {
            c.Add(1);
            JobOf<std::remove_reference_t<F>> jf{ Forward<F>(f) };
            return Concurrency::RunWithCounter(jf.AsJob(), c);
        }
    };
}