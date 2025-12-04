use prm_window::*;

fn main() {
    let desc = WindowDesc { width: 640, height: 480, resizable: true, visible: true };
    let w = create(&desc, None).unwrap();
    show(w).unwrap();
    set_title(w, "Prm.Window Smoke").unwrap();
    let mut spins = 0;
    while spins < 100 {
        if !process_one_message(Some(w)) { spins += 1; }
    }
    destroy(w);
    println!("Window OK");
}

