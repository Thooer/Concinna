use sys_vfs::Vfs;
use prm_file::{FileOpenMode, FileShareMode};
use prm_file::{open, read, size, close, write, stdout_handle};
use wasmtime::{Engine, Module, Store, Linker, Caller, Extern};
use mlua::Lua;
use std::sync::Arc;
use sys_ir::Value;

fn read_text_vfs(vfs: &Vfs, vpath: &str) -> Option<String> {
    let h = vfs.open_read(vpath).ok()?;
    let sz = size(h).ok()? as usize;
    let mut buf = vec![0u8; sz.min(1<<20).max(1)];
    let n = read(h, &mut buf).ok()?;
    let _ = close(h);
    Some(String::from_utf8_lossy(&buf[..n]).to_string())
}

fn out_line(s: &str) { let h = stdout_handle(); let _ = write(h, s.as_bytes()); let _ = write(h, b"\n"); }

pub fn run_lua_file(vfs: &Vfs, vpath: &str) -> bool {
    let src = match read_text_vfs(vfs, vpath) { Some(s) => s, None => return false };
    let lua = Lua::new();
    let pf = match lua.create_function(|_, s: String| { out_line(&s); Ok(()) }) { Ok(f) => f, Err(_) => return false };
    if lua.globals().set("print", pf).is_err() { return false; }
    lua.load(src.as_str()).exec().is_ok()
}

pub fn run_wat_file(vfs: &Vfs, vpath: &str) -> bool {
    let src = match read_text_vfs(vfs, vpath) { Some(s) => s, None => return false };
    let bytes = match wat::parse_bytes(src.as_bytes()) { Ok(b) => b, Err(_) => return false };
    let engine = Engine::default();
    let module = match Module::from_binary(&engine, &bytes) { Ok(m) => m, Err(_) => return false };
    let mut store = Store::new(&engine, ());
    let mut linker = Linker::new(&engine);
    let _ = linker.func_wrap("host", "log", move |mut caller: Caller<'_, ()>, ptr: i32, len: i32| {
        if let Some(Extern::Memory(mem)) = caller.get_export("memory") {
            let mut buf = vec![0u8; len as usize];
            let _ = mem.read(&mut caller, ptr as usize, &mut buf);
            out_line(core::str::from_utf8(&buf).unwrap_or(""));
        }
    });
    let instance = match linker.instantiate(&mut store, &module) { Ok(i) => i, Err(_) => return false };
    let run = match instance.get_func(&mut store, "run") { Some(f) => f, None => return false };
    run.call(&mut store, &[], &mut []).is_ok()
}

pub struct LuaRuntime { pub lua: Lua }

pub fn lua_runtime_new() -> LuaRuntime {
    let lua = Lua::new();
    if let Ok(f) = lua.create_function(|_, s: String| { out_line(&s); Ok(()) }) {
        let _ = lua.globals().set("print", f);
    }
    LuaRuntime { lua }
}

pub fn lua_runtime_register_render(rt: &LuaRuntime, cb: Arc<dyn Fn()>) -> bool {
    let f = rt.lua.create_function(move |_, ()| { cb(); Ok(()) });
    match f { Ok(func) => rt.lua.globals().set("render_triangle", func).is_ok(), Err(_) => false }
}

pub fn lua_runtime_exec_file(rt: &LuaRuntime, vfs: &Vfs, vpath: &str) -> bool {
    let src = match read_text_vfs(vfs, vpath) { Some(s) => s, None => return false };
    rt.lua.load(src.as_str()).exec().is_ok()
}

pub fn lua_runtime_exec_frame(rt: &LuaRuntime, vfs: &Vfs, vpath: &str) -> Option<String> {
    let src = read_text_vfs(vfs, vpath)?;
    if rt.lua.load(src.as_str()).exec().is_err() { return None; }
    let g = rt.lua.globals();
    let f: mlua::Function = g.get("on_frame").ok()?;
    let r: String = f.call(()).ok()?;
    Some(r)
}

pub fn lua_runtime_exec_update(rt: &LuaRuntime, dt: f32, time: f32) {
    let g = rt.lua.globals();
    if let Ok(f) = g.get::<_, mlua::Function>("on_update") {
        let _ = f.call::<(f32, f32), ()>((dt, time));
    }
}

pub fn lua_to_ir(v: mlua::Value) -> Value {
    match v {
        mlua::Value::Nil => Value::Null,
        mlua::Value::Boolean(b) => Value::Bool(b),
        mlua::Value::Integer(i) => Value::Int(i),
        mlua::Value::Number(n) => Value::Float(n),
        mlua::Value::String(s) => Value::String(s.to_str().unwrap_or("").to_string()),
        mlua::Value::Table(t) => {
            // Naive check for array vs map
            let len = t.len().unwrap_or(0);
            if len > 0 && t.contains_key(1).unwrap_or(false) {
                let mut vec = Vec::new();
                for i in 1..=len {
                    if let Ok(val) = t.get(i) {
                        vec.push(lua_to_ir(val));
                    }
                }
                Value::Array(vec)
            } else {
                let mut map = std::collections::HashMap::new();
                for pair in t.pairs::<String, mlua::Value>() {
                    if let Ok((k, v)) = pair {
                        map.insert(k, lua_to_ir(v));
                    }
                }
                Value::Map(map)
            }
        },
        _ => Value::Null,
    }
}

pub fn ir_to_lua<'lua>(lua: &'lua Lua, v: &Value) -> mlua::Result<mlua::Value<'lua>> {
    match v {
        Value::Null => Ok(mlua::Value::Nil),
        Value::Bool(b) => Ok(mlua::Value::Boolean(*b)),
        Value::Int(i) => Ok(mlua::Value::Integer(*i)),
        Value::Float(f) => Ok(mlua::Value::Number(*f)),
        Value::String(s) => Ok(mlua::Value::String(lua.create_string(s)?)),
        Value::Array(arr) => {
            let t = lua.create_table()?;
            for (i, val) in arr.iter().enumerate() {
                t.set(i + 1, ir_to_lua(lua, val)?)?;
            }
            Ok(mlua::Value::Table(t))
        },
        Value::Map(map) => {
            let t = lua.create_table()?;
            for (k, val) in map {
                t.set(k.as_str(), ir_to_lua(lua, val)?)?;
            }
            Ok(mlua::Value::Table(t))
        },
    }
}

pub fn lua_runtime_call_ir(rt: &LuaRuntime, func: &str, args: Value) -> Option<Value> {
    let g = rt.lua.globals();
    let f: mlua::Function = g.get(func).ok()?;
    let lua_args = ir_to_lua(&rt.lua, &args).ok()?;
    let res: mlua::Value = f.call(lua_args).ok()?;
    Some(lua_to_ir(res))
}
