use prm_file::{open, close, read, write, size, stdout_handle, FileOpenMode, FileShareMode};
use prm_io::{path_exists, path_is_directory, path_create_directory};
use prm_time::{sleep_ms, now, delta_seconds};
use prm_window::{create, show, process_one_message, destroy, set_raw_event_callback, WindowDesc};
use std::sync::{atomic::{AtomicBool, Ordering}, Arc, Mutex};
use std::ffi::c_void;
use cap_path::{join, to_windows};
use cap_memory::{SystemMemoryResource, Allocator};
use sys_vfs::Vfs;
use sys_rhi::{Device, FramePacket, InstanceData};
use sys_rendergraph::FrameGraph;
use sys_scripting::{run_lua_file, run_wat_file, lua_runtime_new, lua_runtime_exec_frame, lua_runtime_exec_update, lua_runtime_call_ir};
use sys_memory::StateRingResource;
use cap_math::{Vec3, Quat, Mat4 as CMat4};
use sim_scene::SimWorld;
use sim_schema::EntityId;
use sys_ir::Value;
use std::collections::HashMap;

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

fn read_text_vfs(vfs: &Vfs, vpath: &str) -> Option<String> {
    let h = vfs.open_read(vpath).ok()?;
    let sz = size(h).ok()? as usize;
    let mut buf = vec![0u8; sz.min(1<<20).max(1)];
    let n = read(h, &mut buf).ok()?;
    let _ = close(h);
    Some(String::from_utf8_lossy(&buf[..n]).to_string())
}

fn ensure_dir(path: &str) {
    if !path_exists(path) { let _ = path_create_directory(path); }
}

fn host_print(s: &str) { let sh = stdout_handle(); let _ = write(sh, s.as_bytes()); let _ = write(sh, b"\n"); }

fn cap_to_rhi_mat4(m: CMat4) -> sys_rhi::Mat4 {
    let r = m.rows;
    sys_rhi::Mat4([
        r[0][0], r[0][1], r[0][2], r[0][3],
        r[1][0], r[1][1], r[1][2], r[1][3],
        r[2][0], r[2][1], r[2][2], r[2][3],
        r[3][0], r[3][1], r[3][2], r[3][3],
    ])
}

fn extract(world: &SimWorld<'_>, ids: &[EntityId]) -> FramePacket {
    let mut packet = FramePacket::default();
    for &id in ids {
        if let Some(p) = world.get_position(id) {
             let cm = CMat4::from_trs(p, Quat::identity(), Vec3::new(1.0,1.0,1.0));
             packet.instances.push(InstanceData { model_matrix: cap_to_rhi_mat4(cm), mesh_handle: 0 });
        }
    }
    packet
}

fn main() {
    let cache_path = "Engine/App/cache/last_project.txt";
    let cwd_str = "."; 
    let default_project = format!("{}\\Projects\\Test", cwd_str);
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

    let sys_res = Box::leak(Box::new(SystemMemoryResource));
    let alloc = Allocator::new(sys_res);
    let mut vfs = Vfs::with_capacity(alloc, 8).unwrap();
    let _ = vfs.mount("project", &project_root);
    let engine_root = format!("{}\\Engine", cwd_str);
    let _ = vfs.mount("engine", &engine_root);

    let desc = WindowDesc::default();
    if let Ok(h) = create(&desc, None) {
        static QUIT: AtomicBool = AtomicBool::new(false);
        fn raw_cb(_hwnd: *mut c_void, msg: u32, _wparam: usize, _lparam: isize) {
            if msg == 0x0010 || msg == 0x0002 || msg == 0x0012 { QUIT.store(true, Ordering::SeqCst); }
        }
        set_raw_event_callback(Some(raw_cb));
        let _ = show(h);
        
        // Scripting: Run hello.lua
        let _ = run_lua_file(&vfs, "project:scripts/hello.lua");
        
        let rt = lua_runtime_new();
        let mut ring = StateRingResource::new(128 << 20, 256);
        // Use Arc<Mutex> for SimWorld to share with Lua
        let world_mutex = Arc::new(Mutex::new(SimWorld::new(alloc).unwrap()));
        
        // Register Generic API (IR-based)
        {
            let w = world_mutex.clone();
            let lua = &rt.lua;
            
            let f = lua.create_function(move |lua_ctx, v: mlua::Value| {
                let ir = sys_scripting::lua_to_ir(v);
                let mut sim = w.lock().unwrap();
                match sim.dispatch_ir(&ir) {
                    Ok(res) => sys_scripting::ir_to_lua(lua_ctx, &res),
                    Err(e) => {
                        host_print(&format!("[Script] Error: {}", e));
                        Ok(mlua::Value::Nil)
                    }
                }
            }).unwrap();
            let _ = lua.globals().set("dispatch", f);
        }

        let mut sim_time: f32 = 0.0;
        let mut accumulator: f32 = 0.0;
        let fixed_dt: f32 = 1.0 / 60.0;
        let mut prev_t = now();
        let backend = sys_rhi::BackendKind::Vulkan; // Default to Vulkan
        // Read config to override backend if needed
        let mut dev = Device::new(h, desc.width as u32, desc.height as u32, backend).ok();
        if dev.is_some() { host_print("[RHI] 初始化成功"); } else { host_print("[RHI] 初始化失败"); }
        
        // Load Shaders
        if let Some(vs) = read_text_vfs(&vfs, "project:assets/shaders/simple.vert") {
            if let Some(fl) = read_text_vfs(&vfs, "project:assets/shaders/simple_left.frag") {
                if let Some(fr) = read_text_vfs(&vfs, "project:assets/shaders/simple_right.frag") {
                    if let Some(ref mut ctx) = dev {
                        let desc = sys_rhi::PipelineDesc {
                            vs,
                            fs: fl,
                            fs_right: Some(fr),
                        };
                        if let Ok(_) = ctx.create_pipeline(desc) {
                            host_print("[RHI] Pipeline created successfully");
                        } else {
                            host_print("[RHI] Pipeline creation failed");
                        }
                    }
                } else { host_print("[App] Failed to read simple_right.frag"); }
            } else { host_print("[App] Failed to read simple_left.frag"); }
        } else { host_print("[App] Failed to read simple.vert"); }

        // Init World Entities
        let eid_left;
        let eid_right;
        {
            let mut world = world_mutex.lock().unwrap();
            eid_left = world.spawn_transform(Vec3::new(-0.6, 0.0, 0.0), Quat::identity(), Vec3::new(1.0, 1.0, 1.0)).unwrap();
            eid_right = world.spawn_transform(Vec3::new(0.6, 0.0, 0.0), Quat::identity(), Vec3::new(1.0, 1.0, 1.0)).unwrap();
        }

        // Load Model
        let mut cached_model: Option<sys_rhi::Model> = None;
        if let Some(m) = sys_rhi::load_model_vfs(&vfs, "project:assets/meshes/cube.mesh") {
             if let Some(ref mut ctx) = dev { let _ = ctx.upload_model(&m); }
             cached_model = Some(m);
        }
        
        // Scripting: Load logic script
        let _ = run_lua_file(&vfs, "project:scripts/frame.lua");
        
        // Inject Entity IDs into Lua global context for testing
        {
             let lua = &rt.lua;
             let globals = lua.globals();
             let _ = globals.set("EID_LEFT", eid_left.index());
             let _ = globals.set("EID_RIGHT", eid_right.index());
        }

        loop {
            let _ = process_one_message(Some(h));
            if QUIT.load(Ordering::SeqCst) { break; }
            // Fixed-step logic update
            let cur_t = now();
            let dt = delta_seconds(prev_t, cur_t) as f32;
            prev_t = cur_t;
            accumulator += dt;
            
            // Logic Loop (Sim)
            while accumulator >= fixed_dt {
                ring.begin_frame();
                sim_time += fixed_dt;
                
                // Script Logic: Lua modifies SimWorld via "dispatch"
                lua_runtime_exec_update(&rt, fixed_dt, sim_time);
                
                ring.commit_frame();
                accumulator -= fixed_dt;
            }

            // Render Loop
            if let Some(ref mut ctx) = dev {
                if let Some(ref m) = cached_model {
                    // Extract (Sim -> Packet)
                    let r = 2.2f32; 
                    let eye = [sim_time.sin() * r, 2.0f32, sim_time.cos() * r];
                    let mut packet = {
                         let world = world_mutex.lock().unwrap();
                         extract(&world, &[eid_left, eid_right])
                    };
                    packet.view = sys_rhi::look_at(eye, [0.0,0.0,0.0], [0.0,1.0,0.0]);
                    packet.proj = sys_rhi::perspective(60.0f32.to_radians(), desc.width as f32 / desc.height as f32, 0.1, 10.0);
                    
                    // Render (Packet -> RHI)
                    let _ = ctx.draw_packet(&packet, m);
                    
                    fn pass_depth(dev: &mut Device) { let _ = dev.begin_frame(); let _ = dev.depth_prepass(); }
                    fn pass_color(dev: &mut Device) { let _ = dev.color_pass(); }
                    fn pass_present(dev: &mut Device) { let _ = dev.present(); }

                    let mut fg = FrameGraph::new();
                    fg.add_pass(sys_rendergraph::PassDesc::new("DepthPrePass").execute(pass_depth));
                    fg.add_pass(sys_rendergraph::PassDesc::new("ColorPass").execute(pass_color));
                    fg.add_pass(sys_rendergraph::PassDesc::new("Present").execute(pass_present));
                    
                    if fg.compile() {
                        fg.execute(ctx);
                    }
                }
            }
            
            sleep_ms(1);
        }
        destroy(h);
    }
}
