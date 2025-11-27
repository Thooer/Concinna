export module Prm.Socket:Ops;
import Element;
import Flow;
import Text;
import Paradigm;
import :Types;

export namespace Prm {
    export class Socket {
    public:
        [[nodiscard]] static Expect<SocketHandle> Create(AddressFamily af, SocketType type, Protocol proto) noexcept;
        [[nodiscard]] static Status Close(SocketHandle h) noexcept;

        [[nodiscard]] static Status Bind(SocketHandle h, const SocketAddress& addr) noexcept;
        [[nodiscard]] static Status Listen(SocketHandle h, Int32 backlog) noexcept;
        [[nodiscard]] static Expect<SocketHandle> Accept(SocketHandle h) noexcept;
        [[nodiscard]] static Status Connect(SocketHandle h, const SocketAddress& addr) noexcept;

        [[nodiscard]] static Expect<USize> Send(SocketHandle h, Span<const Byte, DynamicExtent> data) noexcept;
        [[nodiscard]] static Expect<USize> Recv(SocketHandle h, Span<Byte, DynamicExtent> buffer) noexcept;

        [[nodiscard]] static Status Shutdown(SocketHandle h, ShutdownMode how) noexcept;
        [[nodiscard]] static Status SetNonBlocking(SocketHandle h, bool enable) noexcept;
        [[nodiscard]] static Status SetReuseAddr(SocketHandle h, bool enable) noexcept;
        [[nodiscard]] static Status SetNoDelay(SocketHandle h, bool enable) noexcept;

        [[nodiscard]] static Expect<SocketAddress> MakeIPv4(StringView ip, UInt16 port) noexcept;
        [[nodiscard]] static Expect<SocketAddress> MakeIPv6(StringView ip, UInt16 port) noexcept;
    };
}
