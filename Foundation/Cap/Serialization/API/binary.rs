use cap_stream::{FileReader, FileWriter, StreamError};

pub fn write_u32(w: &mut FileWriter, v: u32) -> Result<(), StreamError> {
    let b = [(v & 0xFF) as u8, ((v>>8)&0xFF) as u8, ((v>>16)&0xFF) as u8, ((v>>24)&0xFF) as u8];
    w.write(&b).map(|_| ())
}
pub fn write_u64(w: &mut FileWriter, v: u64) -> Result<(), StreamError> {
    let b = [
        (v & 0xFF) as u8, ((v>>8)&0xFF) as u8, ((v>>16)&0xFF) as u8, ((v>>24)&0xFF) as u8,
        ((v>>32)&0xFF) as u8, ((v>>40)&0xFF) as u8, ((v>>48)&0xFF) as u8, ((v>>56)&0xFF) as u8,
    ];
    w.write(&b).map(|_| ())
}
pub fn write_f32(w: &mut FileWriter, v: f32) -> Result<(), StreamError> { write_u32(w, v.to_bits()) }
pub fn write_bytes(w: &mut FileWriter, b: &[u8]) -> Result<(), StreamError> { w.write(b).map(|_| ()) }

pub fn read_u32(r: &mut FileReader) -> Result<u32, StreamError> { let mut b=[0u8;4]; let _=r.read(&mut b)?; Ok((b[0] as u32)|((b[1] as u32)<<8)|((b[2] as u32)<<16)|((b[3] as u32)<<24)) }
pub fn read_u64(r: &mut FileReader) -> Result<u64, StreamError> { let mut b=[0u8;8]; let _=r.read(&mut b)?; Ok((b[0] as u64)|((b[1] as u64)<<8)|((b[2] as u64)<<16)|((b[3] as u64)<<24)|((b[4] as u64)<<32)|((b[5] as u64)<<40)|((b[6] as u64)<<48)|((b[7] as u64)<<56)) }
pub fn read_f32(r: &mut FileReader) -> Result<f32, StreamError> { Ok(f32::from_bits(read_u32(r)?)) }
pub fn read_exact<'a>(r: &mut FileReader, out: &'a mut [u8]) -> Result<&'a [u8], StreamError> { let n=r.read(out)?; Ok(&out[..n]) }

