# WSI 模块

## 1. 模块定位

* **职责**：提供窗口系统集成原语接口，用于连接图形API与窗口系统。
* **边界**：不处理高级图形渲染或窗口管理，仅提供基础的窗口系统集成功能。
* **外部依赖**：Prm.Window

## 2. 设计

采用平台抽象层设计，提供统一的窗口系统集成接口，当前支持Windows平台实现。

## 3. API

* **Vulkan表面操作**：
  * `CreateVulkanSurface`：创建Vulkan表面。
  * `DestroyVulkanSurface`：销毁Vulkan表面。

* **CPU渲染呈现操作**：
  * `CreateCpuPresent`：创建CPU渲染呈现状态。
  * `DestroyCpuPresent`：销毁CPU渲染呈现状态。
  * `CpuGetBuffer`：获取CPU渲染缓冲区。
  * `CpuGetPitch`：获取CPU渲染缓冲区行间距。
  * `CpuPresent`：呈现CPU渲染内容到窗口。

## 4. Samples

无