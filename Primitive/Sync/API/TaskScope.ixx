export module Prm.Sync:TaskScope;
import Lang.Element;
import Lang.Flow;
import :Cancel;
import <thread>;
import <vector>;

export namespace Prm {
    struct TaskScope {
        std::vector<std::thread> m_threads;
        TaskScope() noexcept = default;
        TaskScope(const TaskScope&) = delete;
        TaskScope& operator=(const TaskScope&) = delete;
        TaskScope(TaskScope&&) noexcept = default;
        TaskScope& operator=(TaskScope&&) noexcept = default;
        ~TaskScope() noexcept { JoinAll(); }

        template<typename F>
        [[nodiscard]] Expect<void> Spawn(F&& f) noexcept {
            m_threads.emplace_back(static_cast<F&&>(f));
            return Expect<void>::Ok(Ok(StatusDomain::System()));
        }

        template<typename F>
        [[nodiscard]] Expect<void> Spawn(F&& f, CancelToken token) noexcept {
            m_threads.emplace_back([fn = static_cast<F&&>(f), token]() mutable { fn(token); });
            return Expect<void>::Ok(Ok(StatusDomain::System()));
        }

        void JoinAll() noexcept {
            for (auto& t : m_threads) { if (t.joinable()) t.join(); }
            m_threads.clear();
        }
    };
}
