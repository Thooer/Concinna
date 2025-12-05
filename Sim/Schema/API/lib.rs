#![allow(non_snake_case)]

#[derive(Clone, Copy, PartialEq, Eq, Hash, PartialOrd, Ord)]
pub struct EntityId(pub u64);

impl EntityId {
    pub const NULL: Self = Self(u64::MAX);
    
    #[inline]
    pub fn new(index: u32, generation: u32) -> Self {
        Self((index as u64) | ((generation as u64) << 32))
    }
    
    #[inline]
    pub fn index(&self) -> u32 {
        self.0 as u32
    }
    
    #[inline]
    pub fn generation(&self) -> u32 {
        (self.0 >> 32) as u32
    }
}

impl Default for EntityId {
    fn default() -> Self { Self::NULL }
}

impl std::fmt::Debug for EntityId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if *self == Self::NULL {
            write!(f, "Entity(NULL)")
        } else {
            write!(f, "Entity({}:{})", self.index(), self.generation())
        }
    }
}

#[derive(Clone, Copy, PartialEq, Eq, Hash, Default)]
pub struct FrameId(pub u64);

#[derive(Clone, Copy, Default)]
pub struct GameTime { pub frame: FrameId, pub dt: f32 }

