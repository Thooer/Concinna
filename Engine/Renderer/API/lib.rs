use sys_rhi::Device;
use prm_window::WindowHandle;
use prm_time::sleep_ms;

pub struct Renderer { dev: Device, w: u32, h: u32 }

impl Renderer {
    pub fn new(hwnd: WindowHandle, w: u32, h: u32) -> Option<Self> {
        if let Ok(d) = Device::new(hwnd, w, h) { Some(Self { dev: d, w, h }) } else { None }
    }
    pub fn run_frame(&mut self) {
        let _ = self.dev.begin_frame();
        let _ = self.dev.color_pass();
        let _ = self.dev.present();
    }
    pub fn run_loop(&mut self, hwnd: WindowHandle) { loop { let _ = prm_window::process_one_message(Some(hwnd)); self.run_frame(); sleep_ms(16); } }
}
