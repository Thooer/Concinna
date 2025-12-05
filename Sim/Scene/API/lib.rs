use std::any::{Any, TypeId};
use cap_containers::{Vector, HashMap};
use cap_memory::{Allocator, MemoryError};
use sim_schema::EntityId;
use sim_component::Transform;
use cap_math::{Vec3, Quat};
use sys_ir::Value;

// --- Internal Traits for Type Erasure & Cloning ---

pub trait Storage<'a>: Send + Sync {
    fn element_type_id(&self) -> TypeId;
    fn as_raw(&self) -> *const ();
    fn as_raw_mut(&mut self) -> *mut ();
    fn push_any(&mut self, component: Box<dyn Any>);
    fn swap_remove(&mut self, index: usize);
    fn fork(&self, alloc: Allocator<'a>) -> Result<Box<dyn Storage<'a> + 'a>, MemoryError>;
    fn len(&self) -> usize;
}

struct ComponentVec<'a, T> {
    data: Vector<'a, T>,
}

impl<'a, T: 'static + Clone + Send + Sync> Storage<'a> for ComponentVec<'a, T> {
    fn element_type_id(&self) -> TypeId { TypeId::of::<T>() }
    fn as_raw(&self) -> *const () { self as *const _ as *const () }
    fn as_raw_mut(&mut self) -> *mut () { self as *mut _ as *mut () }
    
    fn push_any(&mut self, component: Box<dyn Any>) {
        let val = *component.downcast::<T>().expect("Invalid component type");
        self.data.push(val).expect("OOM in push_any"); 
    }
    fn swap_remove(&mut self, index: usize) {
        self.data.swap_remove(index);
    }
    fn fork(&self, alloc: Allocator<'a>) -> Result<Box<dyn Storage<'a> + 'a>, MemoryError> {
        let mut new_vec = Vector::with_capacity(alloc, self.data.capacity())?;
        for i in 0..self.data.len() {
            if let Some(val) = self.data.get(i) {
                new_vec.push(val.clone())?;
            }
        }
        Ok(Box::new(ComponentVec { data: new_vec }))
    }
    fn len(&self) -> usize { self.data.len() }
}

// --- Archetype ---

pub struct Archetype<'a> {
    pub id: u64,
    pub types: Vector<'a, TypeId>,
    pub storages: HashMap<'a, TypeId, Box<dyn Storage<'a> + 'a>>,
    pub entities: Vector<'a, EntityId>,
    alloc: Allocator<'a>,
}

impl<'a> Archetype<'a> {
    pub fn new(alloc: Allocator<'a>, types: Vector<'a, TypeId>) -> Result<Self, MemoryError> {
        Ok(Self {
            id: 0,
            types,
            storages: HashMap::with_capacity(alloc, 16)?,
            entities: Vector::with_capacity(alloc, 16)?,
            alloc,
        })
    }

    pub fn fork(&self, alloc: Allocator<'a>) -> Result<Self, MemoryError> {
        let mut new_storages = HashMap::with_capacity(alloc, self.storages.capacity())?;
        let iter = self.storages.iter();
        for (tid, storage) in iter {
            new_storages.insert(*tid, storage.fork(alloc)?)?;
        }
        
        let mut new_types = Vector::with_capacity(alloc, self.types.len())?;
        for t in self.types.iter() { new_types.push(*t)?; }
        
        let mut new_entities = Vector::with_capacity(alloc, self.entities.len())?;
        for e in self.entities.iter() { new_entities.push(*e)?; }

        Ok(Self {
            id: self.id,
            types: new_types,
            storages: new_storages,
            entities: new_entities,
            alloc,
        })
    }

    pub fn add_storage<T: 'static + Clone + Send + Sync>(&mut self) -> Result<(), MemoryError> {
        let vec: Vector<'_, T> = Vector::with_capacity(self.alloc, 16)?;
        self.storages.insert(TypeId::of::<T>(), Box::new(ComponentVec { data: vec }))?;
        Ok(())
    }
    
    pub fn push_entity(&mut self, id: EntityId) -> Result<(), MemoryError> {
        self.entities.push(id)
    }
    
    pub fn push_component<T: 'static + Clone + Send + Sync>(&mut self, component: T) {
        let tid = TypeId::of::<T>();
        if let Some(storage) = self.storages.get_mut(&tid) {
            storage.push_any(Box::new(component));
        }
    }
    
    pub fn swap_remove(&mut self, row: usize) -> Option<EntityId> {
        if row >= self.entities.len() { return None; }
        let moved = if self.entities.len() > 1 && row < self.entities.len() - 1 {
             self.entities.get(self.entities.len() - 1).copied()
        } else {
             None
        };
        self.entities.swap_remove(row);
        
        for storage in self.storages.values_mut() {
            storage.swap_remove(row);
        }
        
        moved
    }
}

// --- World ---

#[derive(Clone, Copy, Debug)]
struct EntityRecord {
    archetype_idx: usize,
    row: usize,
}

pub struct SimWorld<'a> {
    entities: Vector<'a, Option<EntityRecord>>,
    generations: Vector<'a, u32>,
    free_indices: Vector<'a, u32>,
    pub archetypes: Vector<'a, Archetype<'a>>,
    alloc: Allocator<'a>,
}

impl<'a> SimWorld<'a> {
    pub fn new(alloc: Allocator<'a>) -> Result<Self, MemoryError> {
        Ok(Self {
            entities: Vector::with_capacity(alloc, 256)?,
            generations: Vector::with_capacity(alloc, 256)?,
            free_indices: Vector::with_capacity(alloc, 64)?,
            archetypes: Vector::with_capacity(alloc, 16)?,
            alloc,
        })
    }
    
    pub fn with_capacity(alloc: Allocator<'a>, capacity: usize) -> Result<Self, MemoryError> {
        Ok(Self {
            entities: Vector::with_capacity(alloc, capacity)?,
            generations: Vector::with_capacity(alloc, capacity)?,
            free_indices: Vector::with_capacity(alloc, 64)?,
            archetypes: Vector::with_capacity(alloc, 16)?,
            alloc,
        })
    }
    
    pub fn len(&self) -> usize {
        self.entities.as_slice().iter().filter(|e| e.is_some()).count()
    }
    
    pub fn alloc_entity(&mut self) -> Result<EntityId, MemoryError> {
        if let Some(idx) = self.free_indices.pop() {
            let gen = *self.generations.get(idx as usize).unwrap();
            Ok(EntityId::new(idx, gen))
        } else {
            let idx = self.entities.len() as u32;
            self.entities.push(None)?;
            self.generations.push(0)?;
            Ok(EntityId::new(idx, 0))
        }
    }
    
    pub fn spawn_transform(&mut self, p: Vec3, r: Quat, s: Vec3) -> Result<EntityId, MemoryError> {
        let eid = self.alloc_entity()?;
        let t = Transform::from_trs(p, r, s);
        let tid = TypeId::of::<Transform>();
        let arch_idx = self.find_or_create_archetype(&[tid])?;
        
        {
            let arch = self.archetypes.get_mut(arch_idx).unwrap();
            if arch.storages.get(&tid).is_none() {
                 arch.add_storage::<Transform>()?;
            }
            arch.push_entity(eid)?;
            arch.push_component(t);
        }

        let arch = self.archetypes.get(arch_idx).unwrap();
        let row = arch.entities.len() - 1; 
        
        if let Some(rec) = self.entities.get_mut(eid.index() as usize) {
            *rec = Some(EntityRecord { archetype_idx: arch_idx, row });
        }
        Ok(eid)
    }
    
    fn find_or_create_archetype(&mut self, types: &[TypeId]) -> Result<usize, MemoryError> {
        for (i, arch) in self.archetypes.as_slice().iter().enumerate() {
             if arch.types.as_slice() == types { return Ok(i); }
        }
        
        let idx = self.archetypes.len();
        let mut type_vec = Vector::with_capacity(self.alloc, types.len())?;
        for t in types { type_vec.push(*t)?; }
        
        let arch = Archetype::new(self.alloc, type_vec)?;
        self.archetypes.push(arch)?;
        Ok(idx)
    }
    
    pub fn set_position(&mut self, id: EntityId, p: Vec3) {
        if let Some(t) = self.get_component_mut::<Transform>(id) {
            t.px = p.x; t.py = p.y; t.pz = p.z;
        }
    }
    
    pub fn get_position(&self, id: EntityId) -> Option<Vec3> {
        self.get_component::<Transform>(id).map(|t| t.position())
    }
    
    pub fn get_component<T: 'static + Clone + Send + Sync>(&self, id: EntityId) -> Option<&T> {
        let rec = self.entities.get(id.index() as usize)?.as_ref()?;
        if *self.generations.get(id.index() as usize)? != id.generation() { return None; }
        
        let arch = self.archetypes.get(rec.archetype_idx)?;
        let storage = arch.storages.get(&TypeId::of::<T>())?;
        
        if storage.element_type_id() == TypeId::of::<T>() {
             let vec_storage = unsafe { &*(storage.as_raw() as *const ComponentVec<T>) };
             vec_storage.data.get(rec.row)
        } else {
            None
        }
    }
    
    pub fn get_component_mut<T: 'static + Clone + Send + Sync>(&mut self, id: EntityId) -> Option<&mut T> {
        let rec = self.entities.get(id.index() as usize)?.as_ref()?;
        if *self.generations.get(id.index() as usize)? != id.generation() { return None; }
        
        let arch = self.archetypes.get_mut(rec.archetype_idx)?;
        let storage = arch.storages.get_mut(&TypeId::of::<T>())?;
        
        if storage.element_type_id() == TypeId::of::<T>() {
             let vec_storage = unsafe { &mut *(storage.as_raw_mut() as *mut ComponentVec<T>) };
             vec_storage.data.get_mut(rec.row)
        } else {
            None
        }
    }
    
    pub fn fork(&self, alloc: Allocator<'a>) -> Result<SimWorld<'a>, MemoryError> {
        let mut new_entities = Vector::with_capacity(alloc, self.entities.len())?;
        for e in self.entities.iter() { new_entities.push(*e)?; }
        
        let mut new_gens = Vector::with_capacity(alloc, self.generations.len())?;
        for g in self.generations.iter() { new_gens.push(*g)?; }
        
        let mut new_free = Vector::with_capacity(alloc, self.free_indices.len())?;
        for f in self.free_indices.iter() { new_free.push(*f)?; }
        
        let mut new_archetypes = Vector::with_capacity(alloc, self.archetypes.len())?;
        for a in self.archetypes.iter() { new_archetypes.push(a.fork(alloc)?)?; }
        
        Ok(SimWorld {
            entities: new_entities,
            generations: new_gens,
            free_indices: new_free,
            archetypes: new_archetypes,
            alloc,
        })
    }

    pub fn dispatch_ir(&mut self, cmd: &Value) -> Result<Value, String> {
        if let Value::Map(m) = cmd {
            if let Some(Value::String(op)) = m.get("op") {
                match op.as_str() {
                    "set_pos" => {
                        let id = m.get("id").and_then(|v| v.as_f64()).map(|f| f as u64).unwrap_or(0);
                        let eid = EntityId::new(id as u32, 0);
                        let x = m.get("x").and_then(|v| v.as_f64()).unwrap_or(0.0) as f32;
                        let y = m.get("y").and_then(|v| v.as_f64()).unwrap_or(0.0) as f32;
                        let z = m.get("z").and_then(|v| v.as_f64()).unwrap_or(0.0) as f32;
                        self.set_position(eid, Vec3::new(x, y, z));
                        Ok(Value::Null)
                    },
                    "get_pos" => {
                        let id = m.get("id").and_then(|v| v.as_f64()).map(|f| f as u64).unwrap_or(0);
                        let eid = EntityId::new(id as u32, 0);
                        if let Some(p) = self.get_position(eid) {
                            let mut ret = std::collections::HashMap::new();
                            ret.insert("x".to_string(), Value::Float(p.x as f64));
                            ret.insert("y".to_string(), Value::Float(p.y as f64));
                            ret.insert("z".to_string(), Value::Float(p.z as f64));
                            Ok(Value::Map(ret))
                        } else {
                            Ok(Value::Null)
                        }
                    },
                    _ => Err("Unknown op".to_string())
                }
            } else { Err("Missing op".to_string()) }
        } else { Err("Command must be a map".to_string()) }
    }
}

// --- Legacy Support for FrameCommands ---

#[derive(Clone, Copy)]
pub enum Command { Move(EntityId, Vec3), Spawn(Transform) }

pub struct FrameCommands<'a> { cmds: Vector<'a, Command> }

impl<'a> FrameCommands<'a> {
    pub fn new(alloc: Allocator<'a>, capacity: usize) -> Result<Self, MemoryError> { 
        Ok(Self { cmds: Vector::with_capacity(alloc, capacity)? }) 
    }
    pub fn push(&mut self, c: Command) -> Result<(), MemoryError> { 
        self.cmds.push(c)
    }
    pub fn len(&self) -> usize { self.cmds.len() }
    pub fn is_empty(&self) -> bool { self.cmds.is_empty() }
    pub fn as_slice(&self) -> &[Command] { self.cmds.as_slice() }
}

pub trait System { 
    fn run<'a, 'b>(&self, world: &SimWorld<'a>, out: &mut FrameCommands<'b>); 
}

impl<'a> SimWorld<'a> {
    pub fn read_phase<'b, S: System>(&self, systems: &[S], out: &mut FrameCommands<'b>) {
        for s in systems { s.run(self, out); }
    }
    pub fn resolve_phase<'b>(&mut self, out: &FrameCommands<'b>) -> Result<(), MemoryError> {
        for cmd in out.cmds.as_slice() {
            match cmd {
                Command::Spawn(x) => {
                    let _ = self.spawn_transform(x.position(), x.rotation(), x.scale())?;
                },
                Command::Move(e, dp) => {
                    if let Some(pos) = self.get_position(*e) {
                         let p1 = Vec3 { x: pos.x + dp.x, y: pos.y + dp.y, z: pos.z + dp.z };
                         self.set_position(*e, p1);
                    }
                },
            }
        }
        Ok(())
    }
}
