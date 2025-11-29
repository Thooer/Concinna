module Concurrency;
import Lang;
import :TaskGroup;
import :Counter;
import :Scheduler;
import :Job;

namespace Concurrency {
    TaskGroup::TaskGroup() noexcept { c.Reset(0); }
    TaskGroup::~TaskGroup() noexcept { Concurrency::WaitForCounter(c); }
}