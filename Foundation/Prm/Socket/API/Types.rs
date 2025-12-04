#[derive(Clone, Copy)]
pub struct SocketHandle(pub usize);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SocketError { Failed }
