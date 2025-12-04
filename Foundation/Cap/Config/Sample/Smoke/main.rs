use cap_config::*;
use cap_stream::*;
use cap_path::*;

fn main() {
    let mut tmp = [0u8; 256];
    let n1 = join("./", "cap_config_smoke.ini", &mut tmp).unwrap();
    let mut path_buf = [0u8; 256];
    let n2 = to_windows(core::str::from_utf8(&tmp[..n1]).unwrap(), &mut path_buf).unwrap();
    let path = core::str::from_utf8(&path_buf[..n2]).unwrap();

    let mut w = FileWriter::create(path).unwrap();
    let _ = w.write(b"; demo\n[window]\nwidth=1280\nheight=720\n[render]\nvsync:true\n");
    let _ = w.flush();
    drop(w);

    let cfg = Config::load(path).unwrap();
    let w = cfg.get_u32(Some("window"), "width").unwrap();
    let h = cfg.get_u32(Some("window"), "height").unwrap();
    let v = cfg.get_str(Some("render"), "vsync").unwrap();
    println!("cfg {}x{} vsync={}", w, h, v);
}
