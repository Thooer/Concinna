# Threading 模块

## 1. 模块定位

* **职责**：提供跨平台的线程创建、同步和调度原语接口。
* **边界**：不处理高级线程池或复杂并发模式，仅提供基础线程和同步原语。
* **外部依赖**：无

## 2. 设计

采用平台抽象层设计，提供统一的线程和同步原语接口，支持多种平台实现（Windows、Noop）。

## 3. API

* **线程操作**：
  * `ThreadCreate`：创建线程。
  * `ThreadJoin`：等待线程结束。
  * `ThreadYield`：让出CPU时间片。
  * `ThreadSleepMs`：线程睡眠指定毫秒数。
  * `ThreadSetAffinityMask`：设置线程亲和性掩码。
  * `ThreadSetGroupAffinity`：设置线程组亲和性。

* **互斥锁操作**：
  * `MutexCreate`：创建互斥锁。
  * `MutexDestroy`：销毁互斥锁。
  * `MutexLock`：获取互斥锁。
  * `MutexUnlock`：释放互斥锁。
  * `MutexTryLock`：尝试获取互斥锁。

* **信号量操作**：
  * `SemaphoreCreate`：创建信号量。
  * `SemaphoreDestroy`：销毁信号量。
  * `SemaphoreAcquire`：获取信号量。
  * `SemaphoreRelease`：释放信号量。

* **事件操作**：
  * `EventCreate`：创建事件。
  * `EventDestroy`：销毁事件。
  * `EventWait`：等待事件。
  * `EventSignal`：触发事件。
  * `EventReset`：重置事件。

* **地址等待操作**：
  * `WaitOnAddress`：等待地址值变化。
  * `WakeByAddressSingle`：唤醒单个等待地址的线程。
  * `WakeByAddressAll`：唤醒所有等待地址的线程。

## 4. Samples

无