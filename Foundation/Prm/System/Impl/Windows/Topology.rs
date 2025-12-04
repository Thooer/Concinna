use std::ffi::c_void;
use std::mem::{size_of, MaybeUninit};
use crate::topology::*;

#[repr(C)]
pub struct GROUP_AFFINITY { pub Mask: usize, pub Group: u16, pub Reserved: [u16; 3] }

extern "system" {
    fn GetActiveProcessorGroupCount() -> u16;
    fn GetActiveProcessorCount(GroupNumber: u16) -> u32;
    fn GetNumaHighestNodeNumber(HighestNodeNumber: *mut u32) -> i32;
    fn GetNumaNodeProcessorMaskEx(Node: u16, ProcessorMask: *mut GROUP_AFFINITY) -> i32;
    fn GetLogicalProcessorInformation(Buffer: *mut SYSTEM_LOGICAL_PROCESSOR_INFORMATION, ReturnedLength: *mut u32) -> i32;
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct CACHE_DESCRIPTOR { pub Level: u8, pub Associativity: u8, pub LineSize: u16, pub Size: u32, pub Type: u32 }

#[repr(C)]
#[derive(Clone, Copy)]
pub struct SLPI_ProcessorCore { pub Flags: u8 }

#[repr(C)]
#[derive(Clone, Copy)]
pub struct SLPI_NumaNode { pub NodeNumber: u32, pub Reserved1: usize, pub Reserved2: usize }

#[repr(C)]
#[derive(Clone, Copy)]
pub union SLPI_U {
    pub ProcessorCore: SLPI_ProcessorCore,
    pub NumaNode: SLPI_NumaNode,
    pub Cache: CACHE_DESCRIPTOR,
    pub Reserved: [u64; 2],
}

#[repr(C)]
#[derive(Clone, Copy)]
pub struct SYSTEM_LOGICAL_PROCESSOR_INFORMATION { pub ProcessorMask: usize, pub Relationship: u32, pub U: SLPI_U }

pub fn impl_query_topology() -> SystemTopology {
    let groups_cnt = unsafe { GetActiveProcessorGroupCount() } as usize;
    let mut groups = Vec::with_capacity(groups_cnt);
    for g in 0..groups_cnt { let cnt = unsafe { GetActiveProcessorCount(g as u16) }; groups.push(GroupInfo { group: g as u16, active_count: cnt, mask: if cnt >= size_of::<usize>() as u32 * 8 { usize::MAX } else { ((1usize << cnt) - 1) } }); }
    let mut highest = 0u32; unsafe { let _ = GetNumaHighestNodeNumber(&mut highest as *mut u32); }
    let mut numa_nodes = Vec::with_capacity(highest as usize + 1);
    for n in 0..=highest { let mut ga = GROUP_AFFINITY { Mask: 0, Group: 0, Reserved: [0;3] }; let ok = unsafe { GetNumaNodeProcessorMaskEx(n as u16, &mut ga as *mut GROUP_AFFINITY) } != 0; if ok { numa_nodes.push(NumaNode { node_id: n as u16, processors: vec![ProcessorMask { group: ga.Group, mask: ga.Mask }] }); } }
    let total: u32 = groups.iter().map(|g| g.active_count).sum();
    let mut caches: Vec<CacheInfo> = Vec::new();
    unsafe {
        let mut len: u32 = 0;
        let _ = GetLogicalProcessorInformation(std::ptr::null_mut(), &mut len as *mut u32);
        if len > 0 {
            let count = len as usize / std::mem::size_of::<SYSTEM_LOGICAL_PROCESSOR_INFORMATION>();
            let mut buf: Vec<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> = vec![std::mem::zeroed(); count];
            if GetLogicalProcessorInformation(buf.as_mut_ptr(), &mut len as *mut u32) != 0 {
                for i in 0..count {
                    let rel = buf[i].Relationship;
                    if rel == 3 {
                        let cd = buf[i].U.Cache;
                        caches.push(CacheInfo { level: cd.Level, line_size: cd.LineSize, size: cd.Size as usize });
                    }
                }
            }
        }
    }
    SystemTopology { processor_count: total, groups, numa_nodes, caches }
}
