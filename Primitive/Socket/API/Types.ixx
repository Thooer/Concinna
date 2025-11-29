export module Prm.Socket:Types;
import Lang.Element;
import Lang.Paradigm;

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
}
