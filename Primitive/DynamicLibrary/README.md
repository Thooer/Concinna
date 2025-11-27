# DynamicLibrary 模块

## 1. 模块定位

* **职责**：提供跨平台的动态库加载与符号查询接口，封装C ABI为类型安全接口。
* **边界**：不处理复杂的库依赖管理或版本控制。
* **外部依赖**：无

## 2. 设计

采用平台抽象层设计，提供统一的动态库操作接口，当前支持Windows平台实现。

## 3. API

* `DynamicLibrary::Load`：加载指定路径的动态库。
* `DynamicLibrary::Unload`：卸载已加载的动态库。
* `DynamicLibrary::Find`：在动态库中查找指定名称的符号地址。

## 4. Samples

无