use prm_file::{FileHandle, FileOpenMode, FileShareMode, SeekOrigin};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum StreamError { Failed, BufferTooSmall, Unsupported }

pub struct FileReader { h: FileHandle }
pub struct FileWriter { h: FileHandle }
pub struct FileReadWrite { h: FileHandle }

impl FileReader {
    pub fn open(path: &str) -> Result<Self, StreamError> { match prm_file::open(path, FileOpenMode::Read, FileShareMode::Read) { Ok(h) => Ok(Self { h }), Err(_) => Err(StreamError::Failed) } }
    pub fn read(&mut self, out: &mut [u8]) -> Result<usize, StreamError> { prm_file::read(self.h, out).map_err(|_| StreamError::Failed) }
    pub fn size(&self) -> Result<u64, StreamError> { prm_file::size(self.h).map_err(|_| StreamError::Failed) }
    pub fn seek(&mut self, offset: i64, origin: SeekOrigin) -> Result<u64, StreamError> { prm_file::seek(self.h, offset, origin).map_err(|_| StreamError::Failed) }
}

impl Drop for FileReader { fn drop(&mut self) { let _ = prm_file::close(self.h); } }

impl FileWriter {
    pub fn create(path: &str) -> Result<Self, StreamError> { match prm_file::open(path, FileOpenMode::Create, FileShareMode::ReadWrite) { Ok(h) => Ok(Self { h }), Err(_) => Err(StreamError::Failed) } }
    pub fn write(&mut self, src: &[u8]) -> Result<usize, StreamError> { prm_file::write(self.h, src).map_err(|_| StreamError::Failed) }
    pub fn flush(&mut self) -> Result<(), StreamError> { prm_file::flush(self.h).map_err(|_| StreamError::Failed) }
}

impl Drop for FileWriter { fn drop(&mut self) { let _ = prm_file::close(self.h); } }

impl FileReadWrite {
    pub fn open(path: &str) -> Result<Self, StreamError> { match prm_file::open(path, FileOpenMode::ReadWrite, FileShareMode::ReadWrite) { Ok(h) => Ok(Self { h }), Err(_) => Err(StreamError::Failed) } }
    pub fn read(&mut self, out: &mut [u8]) -> Result<usize, StreamError> { prm_file::read(self.h, out).map_err(|_| StreamError::Failed) }
    pub fn write(&mut self, src: &[u8]) -> Result<usize, StreamError> { prm_file::write(self.h, src).map_err(|_| StreamError::Failed) }
    pub fn size(&self) -> Result<u64, StreamError> { prm_file::size(self.h).map_err(|_| StreamError::Failed) }
    pub fn seek(&mut self, offset: i64, origin: SeekOrigin) -> Result<u64, StreamError> { prm_file::seek(self.h, offset, origin).map_err(|_| StreamError::Failed) }
    pub fn flush(&mut self) -> Result<(), StreamError> { prm_file::flush(self.h).map_err(|_| StreamError::Failed) }
}

impl Drop for FileReadWrite { fn drop(&mut self) { let _ = prm_file::close(self.h); } }

