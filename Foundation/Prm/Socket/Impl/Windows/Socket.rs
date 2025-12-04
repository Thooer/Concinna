#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals)]
use std::ffi::c_void;
type SOCKET = usize;

#[repr(C)]
struct WSADATA { data: [u8; 400] }

#[repr(C)]
struct SOCKADDR_IN {
    sin_family: u16,
    sin_port: u16,
    sin_addr: u32,
    sin_zero: [u8; 8],
}

const AF_INET: i32 = 2;
const SOCK_STREAM: i32 = 1;
const IPPROTO_TCP: i32 = 6;
const INVALID_SOCKET: usize = !0;
const SOCKET_ERROR: i32 = -1;

#[link(name = "ws2_32")]
extern "system" {
    fn WSAStartup(wVersionRequested: u16, lpWSAData: *mut WSADATA) -> i32;
    fn WSACleanup() -> i32;
    fn socket(af: i32, kind: i32, protocol: i32) -> SOCKET;
    fn closesocket(s: SOCKET) -> i32;
    fn bind(s: SOCKET, name: *const SOCKADDR_IN, namelen: i32) -> i32;
    fn listen(s: SOCKET, backlog: i32) -> i32;
    fn accept(s: SOCKET, addr: *mut c_void, addrlen: *mut i32) -> SOCKET;
    fn connect(s: SOCKET, name: *const SOCKADDR_IN, namelen: i32) -> i32;
    fn send(s: SOCKET, buf: *const u8, len: i32, flags: i32) -> i32;
    fn recv(s: SOCKET, buf: *mut u8, len: i32, flags: i32) -> i32;
    fn htons(hostshort: u16) -> u16;
    fn inet_addr(cp: *const i8) -> u32;
}

use crate::ops::*;

pub fn impl_init() -> Result<(), SocketError> { let mut d = WSADATA { data: [0; 400] }; let r = unsafe { WSAStartup(0x202, &mut d as *mut WSADATA) }; if r != 0 { Err(SocketError::Failed) } else { Ok(()) } }
pub fn impl_cleanup() { unsafe { WSACleanup(); } }

pub fn impl_tcp_socket() -> Result<SocketHandle, SocketError> { let s = unsafe { socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) }; if s == INVALID_SOCKET { Err(SocketError::Failed) } else { Ok(SocketHandle(s)) } }

fn sockaddr_loopback(port: u16) -> SOCKADDR_IN {
    static LOOPBACK: [i8; 10] = [
        '1' as i8,'2' as i8,'7' as i8,'.' as i8,
        '0' as i8,'.' as i8,
        '0' as i8,'.' as i8,
        '1' as i8,
        0i8
    ];
    let ip = unsafe { inet_addr(LOOPBACK.as_ptr()) };
    SOCKADDR_IN { sin_family: AF_INET as u16, sin_port: unsafe { htons(port) }, sin_addr: ip, sin_zero: [0; 8] }
}

pub fn impl_bind_loopback(h: SocketHandle, port: u16) -> Result<u16, SocketError> {
    let mut p = port;
    if p == 0 { p = 0xC000; }
    let addr = sockaddr_loopback(p);
    let r = unsafe { bind(h.0, &addr as *const SOCKADDR_IN, std::mem::size_of::<SOCKADDR_IN>() as i32) };
    if r == SOCKET_ERROR { Err(SocketError::Failed) } else { Ok(p) }
}

pub fn impl_listen(h: SocketHandle) -> Result<(), SocketError> { let r = unsafe { listen(h.0, 1) }; if r == SOCKET_ERROR { Err(SocketError::Failed) } else { Ok(()) } }
pub fn impl_accept(h: SocketHandle) -> Result<SocketHandle, SocketError> { let s = unsafe { accept(h.0, std::ptr::null_mut(), std::ptr::null_mut()) }; if s == INVALID_SOCKET { Err(SocketError::Failed) } else { Ok(SocketHandle(s)) } }
pub fn impl_connect_loopback(h: SocketHandle, port: u16) -> Result<(), SocketError> { let addr = sockaddr_loopback(port); let r = unsafe { connect(h.0, &addr as *const SOCKADDR_IN, std::mem::size_of::<SOCKADDR_IN>() as i32) }; if r == SOCKET_ERROR { Err(SocketError::Failed) } else { Ok(()) } }
pub fn impl_send(h: SocketHandle, data: &[u8]) -> Result<usize, SocketError> { let n = unsafe { send(h.0, data.as_ptr(), data.len() as i32, 0) }; if n == SOCKET_ERROR { Err(SocketError::Failed) } else { Ok(n as usize) } }
pub fn impl_recv(h: SocketHandle, buf: &mut [u8]) -> Result<usize, SocketError> { let n = unsafe { recv(h.0, buf.as_mut_ptr(), buf.len() as i32, 0) }; if n == SOCKET_ERROR { Err(SocketError::Failed) } else { Ok(n as usize) } }
pub fn impl_close(h: SocketHandle) { unsafe { closesocket(h.0); } }
