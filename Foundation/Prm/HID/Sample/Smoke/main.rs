use prm_window::*;
use prm_hid::*;

fn raw_cb(hwnd: *mut std::ffi::c_void, msg: u32, wparam: usize, lparam: isize) { on_raw_event(hwnd, msg, wparam, lparam) }

fn main() {
    let w = create(&WindowDesc::default(), None).unwrap();
    show(w).unwrap();
    register_devices(w.0).unwrap();
    set_raw_event_callback(Some(raw_cb));
    let mut idle = 0;
    while idle < 500 {
        if !process_one_message(Some(w)) { idle += 1; }
        let mut evs = [HidEvent::MouseMove(0, 0); 32];
        let n = take_events(&mut evs);
        for i in 0..n {
            match evs[i] {
                HidEvent::KeyDown(v) => println!("KeyDown {}", v),
                HidEvent::KeyUp(v) => println!("KeyUp {}", v),
                HidEvent::MouseMove(dx, dy) => println!("Move {} {}", dx, dy),
                HidEvent::MouseLeftDown => println!("LDown"),
                HidEvent::MouseLeftUp => println!("LUp"),
                HidEvent::MouseRightDown => println!("RDown"),
                HidEvent::MouseRightUp => println!("RUp"),
                HidEvent::MouseMiddleDown => println!("MDown"),
                HidEvent::MouseMiddleUp => println!("MUp"),
                HidEvent::MouseWheel(d) => println!("Wheel {}", d),
            }
        }
    }
    destroy(w);
}

