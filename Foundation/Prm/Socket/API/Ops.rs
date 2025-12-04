#[path = "Types.rs"]
mod types;
pub use types::*;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Socket.rs"]
mod backend;

pub fn init() -> Result<(), SocketError> { backend::impl_init() }
pub fn cleanup() { backend::impl_cleanup() }
pub fn tcp_socket() -> Result<SocketHandle, SocketError> { backend::impl_tcp_socket() }
pub fn bind_loopback(h: SocketHandle, port: u16) -> Result<u16, SocketError> { backend::impl_bind_loopback(h, port) }
pub fn listen(h: SocketHandle) -> Result<(), SocketError> { backend::impl_listen(h) }
pub fn accept(h: SocketHandle) -> Result<SocketHandle, SocketError> { backend::impl_accept(h) }
pub fn connect_loopback(h: SocketHandle, port: u16) -> Result<(), SocketError> { backend::impl_connect_loopback(h, port) }
pub fn send(h: SocketHandle, data: &[u8]) -> Result<usize, SocketError> { backend::impl_send(h, data) }
pub fn recv(h: SocketHandle, buf: &mut [u8]) -> Result<usize, SocketError> { backend::impl_recv(h, buf) }
pub fn close(h: SocketHandle) { backend::impl_close(h) }

