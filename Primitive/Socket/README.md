# Socket 模块

## 1. 模块定位

* **职责**：提供跨平台的套接字网络编程原语接口。
* **边界**：不处理高级网络协议、连接池或复杂网络拓扑。
* **外部依赖**：无

## 2. 设计

采用平台抽象层设计，提供统一的套接字操作接口，支持多种平台实现（Windows、Noop）。

## 3. API

* `Socket::Create`：创建套接字。
* `Socket::Close`：关闭套接字。
* `Socket::Bind`：绑定套接字到地址。
* `Socket::Listen`：开始监听连接。
* `Socket::Accept`：接受客户端连接。
* `Socket::Connect`：连接到远程主机。
* `Socket::Send`：发送数据。
* `Socket::Recv`：接收数据。
* `Socket::Shutdown`：关闭套接字的读或写操作。
* `Socket::SetNonBlocking`：设置套接字为非阻塞模式。
* `Socket::SetReuseAddr`：设置地址重用选项。
* `Socket::SetNoDelay`：设置TCP_NODELAY选项。
* `Socket::MakeIPv4`：创建IPv4地址。
* `Socket::MakeIPv6`：创建IPv6地址。

## 4. Samples

无