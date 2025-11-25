module;
module Platform;

import Prm;
import :Socket;

namespace Platform {
    Expect<SocketHandle> Socket::Create(AddressFamily, SocketType, Protocol) noexcept {
        return Expect<SocketHandle>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Status Socket::Close(SocketHandle) noexcept {
        return Err(StatusDomain::System(), StatusCode::Unsupported);
    }
    Status Socket::Bind(SocketHandle, const SocketAddress&) noexcept {
        return Err(StatusDomain::System(), StatusCode::Unsupported);
    }
    Status Socket::Listen(SocketHandle, Int32) noexcept {
        return Err(StatusDomain::System(), StatusCode::Unsupported);
    }
    Expect<SocketHandle> Socket::Accept(SocketHandle) noexcept {
        return Expect<SocketHandle>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Status Socket::Connect(SocketHandle, const SocketAddress&) noexcept {
        return Err(StatusDomain::System(), StatusCode::Unsupported);
    }
    Expect<USize> Socket::Send(SocketHandle, Span<const Byte, DynamicExtent>) noexcept {
        return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Expect<USize> Socket::Recv(SocketHandle, Span<Byte, DynamicExtent>) noexcept {
        return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Status Socket::Shutdown(SocketHandle, ShutdownMode) noexcept {
        return Err(StatusDomain::System(), StatusCode::Unsupported);
    }
    Status Socket::SetNonBlocking(SocketHandle, bool) noexcept {
        return Err(StatusDomain::System(), StatusCode::Unsupported);
    }
    Status Socket::SetReuseAddr(SocketHandle, bool) noexcept {
        return Err(StatusDomain::System(), StatusCode::Unsupported);
    }
    Status Socket::SetNoDelay(SocketHandle, bool) noexcept {
        return Err(StatusDomain::System(), StatusCode::Unsupported);
    }
    Expect<SocketAddress> Socket::MakeIPv4(StringView, UInt16) noexcept {
        return Expect<SocketAddress>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
    Expect<SocketAddress> Socket::MakeIPv6(StringView, UInt16) noexcept {
        return Expect<SocketAddress>::Err(Err(StatusDomain::System(), StatusCode::Unsupported));
    }
}

