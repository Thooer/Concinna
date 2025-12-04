use crate::*;
use sys_vfs::Vfs;
use prm_file::{read, size, close};

pub struct Buffer;
pub struct Texture;

pub struct Model { pub vertices: Vec<[f32;3]>, pub indices: Vec<[u32;3]> }

fn read_text_vfs(vfs: &Vfs, vpath: &str) -> Option<String> {
    let h = vfs.open_read(vpath).ok()?;
    let sz = size(h).ok()? as usize;
    let mut buf = vec![0u8; sz.min(1<<20).max(1)];
    let n = read(h, &mut buf).ok()?;
    let _ = close(h);
    Some(String::from_utf8_lossy(&buf[..n]).to_string())
}

pub fn load_model_vfs(vfs: &Vfs, vpath: &str) -> Option<Model> {
    let s = read_text_vfs(vfs, vpath)?;
    let mut vs: Vec<[f32;3]> = Vec::new();
    let mut is_: Vec<[u32;3]> = Vec::new();
    for line in s.lines() {
        let t = line.trim();
        if t.is_empty() { continue; }
        let mut parts = t.split_whitespace();
        match parts.next() {
            Some("v") => {
                let x = parts.next()?.parse::<f32>().ok()?;
                let y = parts.next()?.parse::<f32>().ok()?;
                let z = parts.next()?.parse::<f32>().ok()?;
                vs.push([x,y,z]);
            }
            Some("i") => {
                let a = parts.next()?.parse::<u32>().ok()?;
                let b = parts.next()?.parse::<u32>().ok()?;
                let c = parts.next()?.parse::<u32>().ok()?;
                is_.push([a,b,c]);
            }
            _ => {}
        }
    }
    Some(Model { vertices: vs, indices: is_ })
}

pub fn raster_model_rgba(width: u32, height: u32, model: &Model) -> Vec<u8> {
    let w = width as i32; let h = height as i32; let pitch = (w as usize) * 4;
    let mut img = vec![0u8; (pitch * h as usize)];
    for y in 0..h as usize { let row = &mut img[y*pitch..(y+1)*pitch]; for x in 0..w as usize { let o = x*4; row[o+0]=16; row[o+1]=16; row[o+2]=16; row[o+3]=255; } }
    for tri in &model.indices {
        let a = model.vertices[tri[0] as usize];
        let b = model.vertices[tri[1] as usize];
        let c = model.vertices[tri[2] as usize];
        let ax = ((a[0]*0.5+0.5)*(w as f32)) as i32; let ay = ((-a[1]*0.5+0.5)*(h as f32)) as i32;
        let bx = ((b[0]*0.5+0.5)*(w as f32)) as i32; let by = ((-b[1]*0.5+0.5)*(h as f32)) as i32;
        let cx = ((c[0]*0.5+0.5)*(w as f32)) as i32; let cy = ((-c[1]*0.5+0.5)*(h as f32)) as i32;
        let min_x = core::cmp::max(0, core::cmp::min(ax, core::cmp::min(bx, cx)));
        let max_x = core::cmp::min(w-1, core::cmp::max(ax, core::cmp::max(bx, cx)));
        let min_y = core::cmp::max(0, core::cmp::min(ay, core::cmp::min(by, cy)));
        let max_y = core::cmp::min(h-1, core::cmp::max(ay, core::cmp::max(by, cy)));
        let area = (bx - ax) * (cy - ay) - (cx - ax) * (by - ay);
        if area == 0 { continue; }
        for y in min_y..=max_y {
            for x in min_x..=max_x {
                let w0 = (bx - ax) * (y - ay) - (by - ay) * (x - ax);
                let w1 = (cx - bx) * (y - by) - (cy - by) * (x - bx);
                let w2 = (ax - cx) * (y - cy) - (ay - cy) * (x - cx);
                let inside = (w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0);
                if inside {
                    let o = (y as usize)*pitch + (x as usize)*4;
                    img[o+0] = 64; img[o+1] = 192; img[o+2] = 64; img[o+3] = 255;
                }
            }
        }
    }
    img
}

#[derive(Clone, Copy)]
pub struct Mat4(pub [f32;16]);

fn dot3(a: [f32;3], b: [f32;3]) -> f32 { a[0]*b[0] + a[1]*b[1] + a[2]*b[2] }
fn sub3(a: [f32;3], b: [f32;3]) -> [f32;3] { [a[0]-b[0], a[1]-b[1], a[2]-b[2]] }
fn cross(a: [f32;3], b: [f32;3]) -> [f32;3] { [a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]] }
fn norm(a: [f32;3]) -> [f32;3] { let l = (dot3(a,a)).sqrt(); if l>0.0 { [a[0]/l,a[1]/l,a[2]/l] } else { a } }

pub fn perspective(fov_rad: f32, aspect: f32, zn: f32, zf: f32) -> Mat4 {
    let f = 1.0 / (0.5 * fov_rad).tan();
    let nf = 1.0 / (zn - zf);
    Mat4([
        f/aspect, 0.0, 0.0, 0.0,
        0.0, f, 0.0, 0.0,
        0.0, 0.0, (zf+zn)*nf, (2.0*zf*zn)*nf,
        0.0, 0.0, -1.0, 0.0
    ])
}

pub fn look_at(eye: [f32;3], center: [f32;3], up: [f32;3]) -> Mat4 {
    let f = norm(sub3(center, eye));
    let s = norm(cross(f, up));
    let u = cross(s, f);
    Mat4([
        s[0], s[1], s[2], -dot3(s, eye),
        u[0], u[1], u[2], -dot3(u, eye),
        -f[0], -f[1], -f[2], dot3(f, eye),
        0.0, 0.0, 0.0, 1.0
    ])
}

pub fn look_at_vk(eye: [f32;3], center: [f32;3], up: [f32;3]) -> Mat4 {
    let f = norm(sub3(center, eye));
    let s = norm(cross(up, f));
    let u = cross(f, s);
    Mat4([
        s[0], s[1], s[2], -dot3(s, eye),
        u[0], u[1], u[2], -dot3(u, eye),
        f[0], f[1], f[2], -dot3(f, eye),
        0.0, 0.0, 0.0, 1.0
    ])
}

pub fn mul(a: Mat4, b: Mat4) -> Mat4 {
    let mut r = [0.0f32;16];
    for i in 0..4 { for j in 0..4 {
        r[i*4+j] = a.0[i*4+0]*b.0[0*4+j] + a.0[i*4+1]*b.0[1*4+j] + a.0[i*4+2]*b.0[2*4+j] + a.0[i*4+3]*b.0[3*4+j];
    }}
    Mat4(r)
}

fn transform_point(m: Mat4, p: [f32;3]) -> [f32;4] {
    let x = p[0]; let y = p[1]; let z = p[2];
    let r0 = m.0[0]*x + m.0[1]*y + m.0[2]*z + m.0[3];
    let r1 = m.0[4]*x + m.0[5]*y + m.0[6]*z + m.0[7];
    let r2 = m.0[8]*x + m.0[9]*y + m.0[10]*z + m.0[11];
    let r3 = m.0[12]*x + m.0[13]*y + m.0[14]*z + m.0[15];
    [r0,r1,r2,r3]
}

pub fn perspective_vk(fov_rad: f32, aspect: f32, zn: f32, zf: f32) -> Mat4 {
    let f = 1.0 / (0.5 * fov_rad).tan();
    let nf = 1.0 / (zf - zn);
    Mat4([
        f/aspect, 0.0, 0.0, 0.0,
        0.0, f, 0.0, 0.0,
        0.0, 0.0, zf*nf, -zn*zf*nf,
        0.0, 0.0, 1.0, 0.0
    ])
}

pub fn raster_cubes_rgba(width: u32, height: u32, model: &Model, angle_rad: f32) -> Vec<u8> {
    let w = width as i32; let h = height as i32; let pitch = (w as usize) * 4;
    let mut img = vec![32u8; (pitch * h as usize)];
    for y in 0..h as usize { let row = &mut img[y*pitch..(y+1)*pitch]; for x in 0..w as usize { let o = x*4; row[o+3]=255; } }
    let mut depth = vec![1.0f32; (w*h) as usize];
    let r = 2.2f32; let eye = [angle_rad.sin()*r, 0.8f32, angle_rad.cos()*r];
    let view = look_at(eye, [0.0,0.0,0.0], [0.0,1.0,0.0]);
    let proj = perspective(60.0f32.to_radians(), width as f32 / height as f32, 0.1, 10.0);
    let t_left = Mat4([1.0,0.0,0.0,-0.6, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0]);
    let t_right = Mat4([1.0,0.0,0.0,0.6, 0.0,1.0,0.0,0.0, 0.0,0.0,1.0,0.0, 0.0,0.0,0.0,1.0]);
    let vp = mul(proj, view);
    let mats = [mul(vp, t_left), mul(vp, t_right)];
    let colors = [[64u8,192u8,64u8],[64u8,64u8,192u8]];
    for (ci, mvp) in mats.iter().enumerate() {
        for tri in &model.indices {
            let a4 = transform_point(*mvp, model.vertices[tri[0] as usize]);
            let b4 = transform_point(*mvp, model.vertices[tri[1] as usize]);
            let c4 = transform_point(*mvp, model.vertices[tri[2] as usize]);
            if a4[3]==0.0 || b4[3]==0.0 || c4[3]==0.0 { continue; }
            let ax = a4[0]/a4[3]; let ay = a4[1]/a4[3]; let az = a4[2]/a4[3];
            let bx = b4[0]/b4[3]; let by = b4[1]/b4[3]; let bz = b4[2]/b4[3];
            let cx = c4[0]/c4[3]; let cy = c4[1]/c4[3]; let cz = c4[2]/c4[3];
            if !(ax>=-1.0 && ax<=1.0 && ay>=-1.0 && ay<=1.0) && !(bx>=-1.0 && bx<=1.0 && by>=-1.0 && by<=1.0) && !(cx>=-1.0 && cx<=1.0 && cy>=-1.0 && cy<=1.0) { continue; }
            let sx = |x: f32| ((x*0.5+0.5)*(w as f32)) as i32;
            let sy = |y: f32| ((-y*0.5+0.5)*(h as f32)) as i32;
            let dax = sx(ax); let day = sy(ay); let dbx = sx(bx); let dby = sy(by); let dcx = sx(cx); let dcy = sy(cy);
            let min_x = core::cmp::max(0, core::cmp::min(dax, core::cmp::min(dbx, dcx)));
            let max_x = core::cmp::min(w-1, core::cmp::max(dax, core::cmp::max(dbx, dcx)));
            let min_y = core::cmp::max(0, core::cmp::min(day, core::cmp::min(dby, dcy)));
            let max_y = core::cmp::min(h-1, core::cmp::max(day, core::cmp::max(dby, dcy)));
            let area = (dbx - dax) * (dcy - day) - (dcx - dax) * (dby - day);
            if area <= 0 { continue; }
            let inv_area = 1.0f32 / (area as f32);
            for y in min_y..=max_y {
                for x in min_x..=max_x {
                    let w0 = (dbx - dax) * (y - day) - (dby - day) * (x - dax);
                    let w1 = (dcx - dbx) * (y - dby) - (dcy - dby) * (x - dbx);
                    let w2 = (dax - dcx) * (y - dcy) - (day - dcy) * (x - dcx);
                    if w0>=0 && w1>=0 && w2>=0 {
                        let b0 = w0 as f32 * inv_area; let b1 = w1 as f32 * inv_area; let b2 = w2 as f32 * inv_area;
                        let z_ndc = az*b0 + bz*b1 + cz*b2;
                        let z = z_ndc * 0.5 + 0.5;
                        let idx = (y as usize)*pitch + (x as usize)*4;
                        let di = (y as usize)*(w as usize) + (x as usize);
                        if z < depth[di] {
                            depth[di] = z;
                            img[idx+0] = colors[ci][2];
                            img[idx+1] = colors[ci][1];
                            img[idx+2] = colors[ci][0];
                            img[idx+3] = 255;
                        }
                    }
                }
            }
        }
    }
    img
}
