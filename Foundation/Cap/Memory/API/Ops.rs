mod types;
pub use types::*;
mod resource;
pub use resource::*;
mod allocator;
pub use allocator::*;

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Ops.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
#[path = "../Impl/Generic/Ops.rs"]
mod backend;
