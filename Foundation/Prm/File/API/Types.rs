use std::ffi::c_void;

#[derive(Clone, Copy)]
pub struct FileHandle(pub *mut c_void);

#[derive(Clone, Copy)]
pub enum FileOpenMode { Read, Write, ReadWrite, Append, Create, CreateNew, Truncate }

#[derive(Clone, Copy)]
pub enum FileShareMode { None, Read, Write, ReadWrite, Delete }

#[derive(Clone, Copy)]
pub enum SeekOrigin { Begin, Current, End }

#[derive(Clone, Copy)]
pub enum MapAccess { Read, Write, ReadWrite }

pub struct Mapping { pub address: *mut c_void, pub length: usize, pub native_mapping_handle: *mut c_void }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FileError { Failed, Unsupported, BufferTooSmall }

