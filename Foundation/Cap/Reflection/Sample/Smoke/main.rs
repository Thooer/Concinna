use cap_reflection::*;

struct SampleActor { id: u64, x: f32, y: f32, name: String, visible: bool }

static FIELDS: &[FieldInfo] = &[
    FieldInfo { name: "id", kind: ValueKind::U64 },
    FieldInfo { name: "x", kind: ValueKind::F32 },
    FieldInfo { name: "y", kind: ValueKind::F32 },
    FieldInfo { name: "name", kind: ValueKind::Str },
    FieldInfo { name: "visible", kind: ValueKind::Bool },
];

impl Reflect for SampleActor {
    fn type_name(&self) -> &'static str { "SampleActor" }
    fn fields(&self) -> &'static [FieldInfo] { FIELDS }
    fn get(&self, name: &str) -> Option<Value> {
        match name {
            "id" => Some(Value::U64(self.id)),
            "x" => Some(Value::F32(self.x)),
            "y" => Some(Value::F32(self.y)),
            "name" => Some(Value::Str(self.name.clone())),
            "visible" => Some(Value::Bool(self.visible)),
            _ => None,
        }
    }
    fn set(&mut self, name: &str, v: Value) -> bool {
        match (name, v) {
            ("id", Value::U64(a)) => { self.id = a; true }
            ,("x", Value::F32(a)) => { self.x = a; true }
            ,("y", Value::F32(a)) => { self.y = a; true }
            ,("name", Value::Str(s)) => { self.name = s; true }
            ,("visible", Value::Bool(b)) => { self.visible = b; true }
            ,_ => false,
        }
    }
}

fn main() {
    let mut a = SampleActor { id: 1, x: 0.1, y: 0.2, name: "hero".to_string(), visible: true };
    let mut buf = String::new();
    a.visit(&mut |n, v| { buf.push_str(n); buf.push('='); match v { Value::U32(x)=>buf.push_str(&x.to_string()), Value::U64(x)=>buf.push_str(&x.to_string()), Value::F32(x)=>buf.push_str(&x.to_string()), Value::F64(x)=>buf.push_str(&x.to_string()), Value::Bool(x)=>buf.push_str(if *x{"true"} else {"false"}), Value::Str(s)=>buf.push_str(s) }; buf.push(';'); });
    let _ok = a.set("x", Value::F32(3.14));
    let _ok = a.set("name", Value::Str("mage".to_string()));
    println!("{} {}", a.type_name(), buf);
}

