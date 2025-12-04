use std::ffi::c_void;

#[derive(Clone, Copy)]
pub struct ProcessorMask { pub group: u16, pub mask: usize }

#[derive(Clone)]
pub struct GroupInfo { pub group: u16, pub active_count: u32, pub mask: usize }

#[derive(Clone)]
pub struct NumaNode { pub node_id: u16, pub processors: Vec<ProcessorMask> }

#[derive(Clone)]
pub struct CacheInfo { pub level: u8, pub line_size: u16, pub size: usize }

#[derive(Clone)]
pub struct SystemTopology { pub processor_count: u32, pub groups: Vec<GroupInfo>, pub numa_nodes: Vec<NumaNode>, pub caches: Vec<CacheInfo> }

#[cfg(target_os = "windows")]
#[path = "../Impl/Windows/Topology.rs"]
mod backend;

#[cfg(not(target_os = "windows"))]
mod backend { use super::*; pub fn impl_query_topology() -> SystemTopology { SystemTopology { processor_count: 1, groups: vec![], numa_nodes: vec![], caches: vec![] } } }

pub fn query_topology() -> SystemTopology { backend::impl_query_topology() }

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn topology_basic() {
        let topo = query_topology();
        assert!(topo.processor_count >= 1);
    }
}
