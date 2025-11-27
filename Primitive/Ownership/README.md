# Ownership 模块

## 1. 模块定位

* **职责**：提供内存所有权管理相关的原语接口，包括虚拟内存操作和堆内存操作。
* **边界**：不处理高级内存分配器或内存池实现。
* **外部依赖**：无

## 2. 设计

采用平台抽象层设计，提供统一的内存操作接口，支持多种平台实现（Windows、Noop）。

## 3. API

* `VirtualMemory::Reserve`：预留虚拟内存。
* `VirtualMemory::Commit`：提交虚拟内存。
* `VirtualMemory::Protect`：修改虚拟内存保护属性。
* `VirtualMemory::Decommit`：取消提交虚拟内存。
* `VirtualMemory::Release`：释放虚拟内存。
* `VirtualMemory::PageSize`：获取页面大小。
* `VirtualMemory::AllocationGranularity`：获取分配粒度。
* `VirtualMemory::LargePageSize`：获取大页尺寸。
* `VirtualMemory::ReserveEx`：高级预留虚拟内存，支持NUMA和大页。
* `Heap::Create`：创建堆。
* `Heap::Destroy`：销毁堆。
* `Heap::GetProcessDefault`：获取进程默认堆。
* `Heap::AllocRaw`：从堆中分配原始内存。
* `Heap::FreeRaw`：释放原始堆内存。
* `Heap::Alloc`：从堆中分配对齐内存。
* `Heap::Free`：释放对齐堆内存。
* `Heap::MaximumAlignment`：获取堆支持的最大对齐。

## 4. Samples

无