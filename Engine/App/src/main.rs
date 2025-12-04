use prm_file::{open, close, read, write, size, stdout_handle, FileOpenMode, FileShareMode};
use prm_io::{path_exists, path_is_directory, path_create_directory};
use prm_time::{sleep_ms, now, delta_seconds};
use prm_window::{create, show, process_one_message, destroy, set_raw_event_callback, WindowDesc};
use std::sync::atomic::{AtomicBool, Ordering};
use std::ffi::c_void;
use cap_path::{join, to_windows};
use cap_memory::{SystemMemoryResource, Allocator};
use sys_vfs::Vfs;
use sys_rhi::Device;
use sys_rendergraph::FrameGraph;
use sys_scripting::{run_lua_file, run_wat_file, lua_runtime_new, lua_runtime_exec_frame};

fn read_text(path: &str) -> Option<String> {
    if let Ok(h) = open(path, FileOpenMode::Read, FileShareMode::Read) {
        let sz = size(h).ok()? as usize;
        let mut buf = vec![0u8; sz.min(4096).max(1)];
        let n = read(h, &mut buf).ok()?;
        let _ = close(h);
        Some(String::from_utf8_lossy(&buf[..n]).trim().to_string())
    } else { None }
}

fn write_text(path: &str, s: &str) -> bool {
    if let Ok(h) = open(path, FileOpenMode::Create, FileShareMode::ReadWrite) {
        let _ = write(h, s.as_bytes());
        let _ = close(h);
        true
    } else { false }
}

fn join_path(a: &str, b: &str) -> String {
    let mut tmp = [0u8; 1024];
    let n = join(a, b, &mut tmp).unwrap_or(0);
    let mut w = vec![0u8; n];
    for i in 0..n { w[i] = tmp[i]; }
    let mut out = vec![0u8; n];
    let m = to_windows(core::str::from_utf8(&w).unwrap_or(""), &mut out).unwrap_or(0);
    String::from_utf8_lossy(&out[..m]).to_string()
}

fn ensure_dir(path: &str) {
    if !path_exists(path) { let _ = path_create_directory(path); }
}

fn host_print(s: &str) { let sh = stdout_handle(); let _ = write(sh, s.as_bytes()); let _ = write(sh, b"\n"); }

fn main() {
    let cache_path = "Engine/App/cache/last_project.txt";
    let cwd_str = std::env::current_dir().unwrap().to_str().unwrap().to_string();
    let default_project = format!("{}\\..\\..\\Projects\\Test", cwd_str);
    let proj = read_text(cache_path).unwrap_or_else(|| {
        let _ = write_text(cache_path, &default_project);
        default_project.to_string()
    });
    let project_root = proj;
    let sh = stdout_handle();
    let _ = write(sh, format!("OPEN {}\n", project_root).as_bytes());
    if !path_exists(&project_root) { let _ = path_create_directory(&project_root); }
    if !path_is_directory(&project_root) { return; }

    let config_dir = join_path(&project_root, "config");
    ensure_dir(&config_dir);
    let structure_cfg = join_path(&config_dir, "structure.txt");
    let default_structure = "assets\nscripts\nconfig\ncache\nbuild\nlogs\ntmp\n";
    let content = match read_text(&structure_cfg) { Some(s) if !s.is_empty() => s, _ => { let _ = write_text(&structure_cfg, default_structure); default_structure.to_string() } };
    for dir in content.split_whitespace() {
        let p = join_path(&project_root, dir);
        ensure_dir(&p);
        let _ = write(sh, format!("DIR {}\n", p).as_bytes());
    }

    let mut sys_res = SystemMemoryResource;
    let alloc = Allocator::new(&mut sys_res);
    let mut vfs = Vfs::with_capacity(alloc, 8).unwrap();
    let _ = vfs.mount("project", &project_root);
    let engine_root = format!("{}\\..\\..\\Engine", cwd_str);
    let _ = vfs.mount("engine", &engine_root);

    let desc = WindowDesc::default();
    if let Ok(h) = create(&desc, None) {
        static QUIT: AtomicBool = AtomicBool::new(false);
        fn raw_cb(_hwnd: *mut c_void, msg: u32, _wparam: usize, _lparam: isize) {
            if msg == 0x0010 || msg == 0x0002 || msg == 0x0012 { QUIT.store(true, Ordering::SeqCst); }
        }
        set_raw_event_callback(Some(raw_cb));
        let _ = show(h);
        let lua_ok = run_lua_file(&vfs, "project:scripts/hello.lua");
        let wat_ok = run_wat_file(&vfs, "project:scripts/hello.wat");
        if !lua_ok && !wat_ok { host_print("No scripting output"); }
        
        let rt = lua_runtime_new();
        let mut dev = Device::new(h, desc.width as u32, desc.height as u32).ok();
        if dev.is_some() { host_print("[RHI] 初始化成功"); } else { host_print("[RHI] 初始化失败"); }
        let t0 = now();
        let mut last_path: Option<String> = None;
        let mut cached_model: Option<sys_rhi::Model> = None;
        loop {
            let _ = process_one_message(Some(h));
            if QUIT.load(Ordering::SeqCst) { break; }
            if let Some(path) = lua_runtime_exec_frame(&rt, &vfs, "project:scripts/frame.lua") {
                let need_reload = match &last_path { Some(p) => p != &path, None => true };
                if need_reload {
                    if let Some(m) = sys_rhi::load_model_vfs(&vfs, &path) {
                        cached_model = Some(m);
                        last_path = Some(path.clone());
                        if let Some(ref mut ctx) = dev { let _ = ctx.upload_model(cached_model.as_ref().unwrap()); }
                    }
                }
                if let Some(ref mut ctx) = dev {
                    if let Some(ref m) = cached_model {
                        let t = now();
                        let angle = (delta_seconds(t0, t) as f32) * 0.8;
                        let _ = ctx.update_cubes(m, angle, desc.width as u32, desc.height as u32);
                        let mut fg = FrameGraph::new();
                        fg.add_pass("DepthPrePass");
                        fg.add_pass("ColorPass");
                        fg.add_pass("Present");
                        if fg.compile() {
                            fg.execute_with(|name| {
                                match name {
                                    "DepthPrePass" => { let _ = ctx.begin_frame(); let _ = ctx.depth_prepass(); }
                                    "ColorPass" => { let _ = ctx.color_pass(); }
                                    "Present" => { let _ = ctx.present(); }
                                    _ => {}
                                }
                            });
                        }
                    }
                }
            }
            sleep_ms(16);
        }
        destroy(h);
    }
}
