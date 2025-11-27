# IO 模块

## 1. 模块定位

* **职责**：提供跨平台的文件系统原语接口，包括文件操作和目录操作。
* **边界**：不处理高级文件系统抽象或复杂的文件格式解析。
* **外部依赖**：无

## 2. 设计

采用平台抽象层设计，提供统一的文件系统操作接口，支持多种平台实现（Windows、Noop）。

## 3. API

* `File::Open`：打开文件，返回文件句柄。
* `File::Close`：关闭文件句柄。
* `File::Read`：从文件中读取数据。
* `File::Write`：向文件中写入数据。
* `File::Size`：获取文件大小。
* `File::Seek`：移动文件指针。
* `File::Map`：将文件映射到内存。
* `File::Unmap`：解除文件内存映射。
* `File::Stdout`：获取标准输出句柄。
* `File::Stderr`：获取标准错误句柄。
* `File::ReadAsync`：异步读取文件。
* `File::WriteAsync`：异步写入文件。
* `File::CancelAsync`：取消异步IO操作。
* `File::CheckAsync`：检查异步IO操作状态。
* `Path::Exists`：检查路径是否存在。
* `Path::IsDirectory`：检查路径是否为目录。
* `Path::CreateDirectory`：创建目录。
* `Path::RemoveFile`：删除文件。

## 4. Samples

无