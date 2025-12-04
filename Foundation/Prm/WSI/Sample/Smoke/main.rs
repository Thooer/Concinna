use prm_wsi::*;
use prm_window::*;

fn main() {
    if let Ok(list) = enumerate_adapters() {
        for (i, a) in list.iter().enumerate() {
            println!("{}: {} (vendor {:04x}, device {:04x}), VRAM {} MB, flags {}", i, a.name, a.vendor_id, a.device_id, a.dedicated_video_memory / (1024 * 1024), a.flags);
        }
    }

    let desc = WindowDesc { width: 640, height: 480, resizable: true, visible: true };
    let w = create(&desc, None).unwrap();
    show(w).unwrap();
    set_title(w, "Prm.WSI Smoke").unwrap();

    let cp = create_cpu_present(w, desc.width, desc.height).unwrap();
    let pitch = cpu_get_pitch(cp).unwrap() as usize;
    let buf = cpu_get_buffer(cp).unwrap();
    let pixels = unsafe { std::slice::from_raw_parts_mut(buf, (desc.width * desc.height * 4) as usize) };

    for y in 0..desc.height {
        for x in 0..desc.width {
            let idx = (y as usize) * pitch + (x as usize) * 4;
            pixels[idx + 0] = (x & 0xFF) as u8;
            pixels[idx + 1] = (y & 0xFF) as u8;
            pixels[idx + 2] = 0x80;
            pixels[idx + 3] = 0xFF;
        }
    }

    cpu_present(cp).unwrap();

    let mut spins = 0;
    while spins < 200 {
        if !process_one_message(Some(w)) { spins += 1; }
    }

    destroy_cpu_present(cp).unwrap();
    destroy(w);
    println!("WSI OK");
}
