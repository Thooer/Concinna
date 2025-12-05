use std::collections::HashMap;

#[derive(Clone, Debug, PartialEq)]
pub enum Value {
    Null,
    Bool(bool),
    Int(i64),
    Float(f64),
    String(String),
    Array(Vec<Value>),
    Map(HashMap<String, Value>),
}

impl Value {
    pub fn as_f64(&self) -> Option<f64> {
        match self {
            Value::Float(f) => Some(*f),
            Value::Int(i) => Some(*i as f64),
            _ => None,
        }
    }
    
    pub fn as_str(&self) -> Option<&str> {
        match self {
            Value::String(s) => Some(s),
            _ => None,
        }
    }
}

pub trait Glue {
    fn to_ir(&self) -> Value;
    fn from_ir(v: &Value) -> Option<Self> where Self: Sized;
}
