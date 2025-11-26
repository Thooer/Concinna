module;
export module Prm.Socket;
import Element;
import Flow;
import Text;
import Paradigm;

export namespace Prm {
    struct SocketHandleTag;
    export using SocketHandle = StrongAlias<void*, SocketHandleTag, AliasHash, AliasEquals, AliasCompare>;

    export enum class AddressFamily : UInt16 { IPv4 = 2, IPv6 = 23 };
    export enum class SocketType : UInt32 { Stream = 1, Datagram = 2 };
    export enum class Protocol : UInt32 { TCP = 6, UDP = 17 };
    export enum class ShutdownMode : UInt32 { Read = 0, Write = 1, Both = 2 };

    export struct SocketAddress {
        AddressFamily family{AddressFamily::IPv4};
        UInt32 length{0};
        Byte   storage[64]{};
    };

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
