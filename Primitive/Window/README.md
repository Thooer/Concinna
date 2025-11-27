# Window 模块

## 1. 模块定位

* **职责**：提供跨平台的窗口创建、管理和消息处理原语接口。
* **边界**：不处理高级GUI框架或复杂窗口布局，仅提供基础窗口操作。
* **外部依赖**：无

## 2. 设计

采用平台抽象层设计，提供统一的窗口操作接口，支持多种平台实现（Windows、Noop）。

## 3. API

* `Window::Create`：创建窗口。
* `Window::Destroy`：销毁窗口。
* `Window::Show`：显示窗口。
* `Window::Hide`：隐藏窗口。
* `Window::SetTitle`：设置窗口标题。
* `Window::Resize`：调整窗口大小。
* `Window::SetCursorMode`：设置光标模式（正常、隐藏、锁定）。
* `Window::Native`：获取原生窗口句柄。
* `Window::ProcessOneMessage`：处理一个窗口消息。

## 4. Samples

无