#[derive(Clone, Debug, PartialEq)]
pub enum Value { U32(u32), U64(u64), F32(f32), F64(f64), Bool(bool), Str(String) }

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ValueKind { U32, U64, F32, F64, Bool, Str }

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct FieldInfo { pub name: &'static str, pub kind: ValueKind }

pub trait Reflect {
    fn type_name(&self) -> &'static str;
    fn fields(&self) -> &'static [FieldInfo];
    fn get(&self, name: &str) -> Option<Value>;
    fn set(&mut self, name: &str, v: Value) -> bool;
    fn visit(&self, f: &mut dyn FnMut(&str, &Value)) { for fi in self.fields() { if let Some(v) = self.get(fi.name) { f(fi.name, &v); } } }
}

