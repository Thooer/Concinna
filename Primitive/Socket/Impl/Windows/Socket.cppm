module;
module Prm.Socket;

import :Socket;

namespace Prm {
    extern "C" __declspec(dllimport) int WSAStartup(unsigned short, void*);
    extern "C" __declspec(dllimport) int WSACleanup(void);
    extern "C" __declspec(dllimport) unsigned long long socket(int, int, int);
    extern "C" __declspec(dllimport) int closesocket(unsigned long long);
    extern "C" __declspec(dllimport) int bind(unsigned long long, const void*, int);
    extern "C" __declspec(dllimport) int listen(unsigned long long, int);
    extern "C" __declspec(dllimport) unsigned long long accept(unsigned long long, void*, int*);
    extern "C" __declspec(dllimport) int connect(unsigned long long, const void*, int);
    extern "C" __declspec(dllimport) int send(unsigned long long, const char*, int, int);
    extern "C" __declspec(dllimport) int recv(unsigned long long, char*, int, int);
    extern "C" __declspec(dllimport) int shutdown(unsigned long long, int);
    extern "C" __declspec(dllimport) int ioctlsocket(unsigned long long, long, unsigned long*);
    extern "C" __declspec(dllimport) unsigned short htons(unsigned short);
    extern "C" __declspec(dllimport) int inet_pton(int, const char*, void*);
    extern "C" __declspec(dllimport) int setsockopt(unsigned long long, int, int, const char*, int);

    static constexpr int kAF_INET  = 2;
    static constexpr int kAF_INET6 = 23;
    static constexpr int kSOCK_STREAM = 1;
    static constexpr int kSOCK_DGRAM  = 2;
    static constexpr int kIPPROTO_TCP = 6;
    static constexpr int kIPPROTO_UDP = 17;
    static constexpr int kSOL_SOCKET  = 0xFFFF;
    static constexpr int kSO_REUSEADDR= 0x0004;
    static constexpr int kTCP_NODELAY = 0x0001;
    static constexpr long kFIONBIO    = 0x8004667E;
    static constexpr int kSD_RECEIVE  = 0;
    static constexpr int kSD_SEND     = 1;
    static constexpr int kSD_BOTH     = 2;

    struct SockAddrIn {
        unsigned short sin_family;
        unsigned short sin_port;
        unsigned int   sin_addr;
        unsigned char  sin_zero[8];
    };
    struct SockAddrIn6 {
        unsigned short sin6_family;
        unsigned short sin6_port;
        unsigned int   sin6_flowinfo;
        unsigned char  sin6_addr[16];
        unsigned int   sin6_scope_id;
    };

    Expect<SocketHandle> Socket::Create(AddressFamily af, SocketType type, Protocol proto) noexcept {
        int iaf = (af == AddressFamily::IPv4) ? kAF_INET : kAF_INET6;
        int itype = (type == SocketType::Stream) ? kSOCK_STREAM : kSOCK_DGRAM;
        int iproto = (proto == Protocol::TCP) ? kIPPROTO_TCP : kIPPROTO_UDP;
        unsigned long long s = socket(iaf, itype, iproto);
        if (!s || s == ~0ull) {
            return Expect<SocketHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        return Expect<SocketHandle>::Ok(SocketHandle{reinterpret_cast<void*>(s)});
    }

    Status Socket::Close(SocketHandle h) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        int r = closesocket(s);
        return r == 0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Status Socket::Bind(SocketHandle h, const SocketAddress& addr) noexcept {
        if (!h.Get() || addr.length == 0) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        int r = bind(s, addr.storage, static_cast<int>(addr.length));
        return r == 0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Status Socket::Listen(SocketHandle h, Int32 backlog) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        int r = listen(s, static_cast<int>(backlog));
        return r == 0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Expect<SocketHandle> Socket::Accept(SocketHandle h) noexcept {
        if (!h.Get()) return Expect<SocketHandle>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        unsigned char addrbuf[64]{};
        int len = sizeof(addrbuf);
        unsigned long long c = accept(s, addrbuf, &len);
        if (!c || c == ~0ull) {
            return Expect<SocketHandle>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        }
        return Expect<SocketHandle>::Ok(SocketHandle{reinterpret_cast<void*>(c)});
    }

    Status Socket::Connect(SocketHandle h, const SocketAddress& addr) noexcept {
        if (!h.Get() || addr.length == 0) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        int r = connect(s, addr.storage, static_cast<int>(addr.length));
        return r == 0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Expect<USize> Socket::Send(SocketHandle h, Span<const Byte, DynamicExtent> data) noexcept {
        if (!h.Get()) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        int r = send(s, reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()), 0);
        if (r < 0) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        return Expect<USize>::Ok(static_cast<USize>(r));
    }

    Expect<USize> Socket::Recv(SocketHandle h, Span<Byte, DynamicExtent> buffer) noexcept {
        if (!h.Get()) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        int r = recv(s, reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), 0);
        if (r < 0) return Expect<USize>::Err(Err(StatusDomain::System(), StatusCode::Failed));
        return Expect<USize>::Ok(static_cast<USize>(r));
    }

    Status Socket::Shutdown(SocketHandle h, ShutdownMode how) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        int mode = (how == ShutdownMode::Read) ? kSD_RECEIVE : (how == ShutdownMode::Write ? kSD_SEND : kSD_BOTH);
        int r = shutdown(s, mode);
        return r == 0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Status Socket::SetNonBlocking(SocketHandle h, bool enable) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        unsigned long mode = enable ? 1u : 0u;
        int r = ioctlsocket(s, kFIONBIO, &mode);
        return r == 0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Status Socket::SetReuseAddr(SocketHandle h, bool enable) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        int v = enable ? 1 : 0;
        int r = setsockopt(s, kSOL_SOCKET, kSO_REUSEADDR, reinterpret_cast<const char*>(&v), sizeof(v));
        return r == 0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Status Socket::SetNoDelay(SocketHandle h, bool enable) noexcept {
        if (!h.Get()) return Err(StatusDomain::System(), StatusCode::InvalidArgument);
        unsigned long long s = reinterpret_cast<unsigned long long>(h.Get());
        int v = enable ? 1 : 0;
        int r = setsockopt(s, kIPPROTO_TCP, kTCP_NODELAY, reinterpret_cast<const char*>(&v), sizeof(v));
        return r == 0 ? Ok(StatusDomain::System()) : Err(StatusDomain::System(), StatusCode::Failed);
    }

    Expect<SocketAddress> Socket::MakeIPv4(StringView ip, UInt16 port) noexcept {
        SocketAddress out{};
        out.family = AddressFamily::IPv4;
        out.length = sizeof(SockAddrIn);
        SockAddrIn sa{};
        sa.sin_family = static_cast<unsigned short>(kAF_INET);
        sa.sin_port = htons(port);
        int ok = inet_pton(kAF_INET, reinterpret_cast<const char*>(ip.data()), &sa.sin_addr);
        if (ok != 1) return Expect<SocketAddress>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        const unsigned char* p = reinterpret_cast<const unsigned char*>(&sa);
        for (USize i = 0; i < sizeof(sa); ++i) { out.storage[i] = static_cast<Byte>(p[i]); }
        return Expect<SocketAddress>::Ok(out);
    }

    Expect<SocketAddress> Socket::MakeIPv6(StringView ip, UInt16 port) noexcept {
        SocketAddress out{};
        out.family = AddressFamily::IPv6;
        out.length = sizeof(SockAddrIn6);
        SockAddrIn6 sa{};
        sa.sin6_family = static_cast<unsigned short>(kAF_INET6);
        sa.sin6_port = htons(port);
        int ok = inet_pton(kAF_INET6, reinterpret_cast<const char*>(ip.data()), &sa.sin6_addr);
        if (ok != 1) return Expect<SocketAddress>::Err(Err(StatusDomain::System(), StatusCode::InvalidArgument));
        const unsigned char* p2 = reinterpret_cast<const unsigned char*>(&sa);
        for (USize i = 0; i < sizeof(sa); ++i) { out.storage[i] = static_cast<Byte>(p2[i]); }
        return Expect<SocketAddress>::Ok(out);
    }
}
