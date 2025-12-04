use std::ffi::{c_void, OsStr};
use std::os::windows::ffi::OsStrExt;

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

extern "system" {
    fn CreateFileW(lpFileName: *const u16, dwDesiredAccess: u32, dwShareMode: u32, lpSecurityAttributes: *mut c_void, dwCreationDisposition: u32, dwFlagsAndAttributes: u32, hTemplateFile: *mut c_void) -> *mut c_void;
    fn ReadFile(hFile: *mut c_void, lpBuffer: *mut c_void, nNumberOfBytesToRead: u32, lpNumberOfBytesRead: *mut u32, lpOverlapped: *mut c_void) -> i32;
    fn WriteFile(hFile: *mut c_void, lpBuffer: *const c_void, nNumberOfBytesToWrite: u32, lpNumberOfBytesWritten: *mut u32, lpOverlapped: *mut c_void) -> i32;
    fn CloseHandle(hObject: *mut c_void) -> i32;
    fn SetFilePointerEx(hFile: *mut c_void, liDistanceToMove: i64, lpNewFilePointer: *mut i64, dwMoveMethod: u32) -> i32;
    fn GetFileSizeEx(hFile: *mut c_void, lpFileSize: *mut i64) -> i32;
    fn GetStdHandle(nStdHandle: i32) -> *mut c_void;

    fn CreateFileMappingW(hFile: *mut c_void, lpFileMappingAttributes: *mut c_void, flProtect: u32, dwMaximumSizeHigh: u32, dwMaximumSizeLow: u32, lpName: *const u16) -> *mut c_void;
    fn MapViewOfFile(hFileMappingObject: *mut c_void, dwDesiredAccess: u32, dwFileOffsetHigh: u32, dwFileOffsetLow: u32, dwNumberOfBytesToMap: usize) -> *mut c_void;
    fn UnmapViewOfFile(lpBaseAddress: *mut c_void) -> i32;
    fn FlushViewOfFile(lpBaseAddress: *mut c_void, dwNumberOfBytesToFlush: usize) -> i32;

    fn GetFileAttributesW(lpFileName: *const u16) -> u32;
    fn CreateDirectoryW(lpPathName: *const u16, lpSecurityAttributes: *mut c_void) -> i32;
    fn DeleteFileW(lpFileName: *const u16) -> i32;
}

const GENERIC_READ: u32 = 0x8000_0000;
const GENERIC_WRITE: u32 = 0x4000_0000;
const FILE_SHARE_READ: u32 = 0x00000001;
const FILE_SHARE_WRITE: u32 = 0x00000002;
const FILE_SHARE_DELETE: u32 = 0x00000004;
const CREATE_NEW: u32 = 1;
const OPEN_EXISTING: u32 = 3;
const OPEN_ALWAYS: u32 = 4;
const TRUNCATE_EXISTING: u32 = 5;
const FILE_ATTRIBUTE_NORMAL: u32 = 0x00000080;
const STD_OUTPUT_HANDLE: i32 = -11;
const STD_ERROR_HANDLE: i32 = -12;
const FILE_ATTRIBUTE_DIRECTORY: u32 = 0x00000010;

const PAGE_READONLY: u32 = 0x02;
const PAGE_READWRITE: u32 = 0x04;
const FILE_MAP_READ: u32 = 0x0004;
const FILE_MAP_WRITE: u32 = 0x0002;

fn wide(s: &str) -> Vec<u16> { OsStr::new(s).encode_wide().chain(std::iter::once(0)).collect() }

pub fn open(path: &str, mode: FileOpenMode, share: FileShareMode) -> Result<FileHandle, ()> {
    let w = wide(path);
    let access = match mode { FileOpenMode::Read => GENERIC_READ, FileOpenMode::Write => GENERIC_WRITE, FileOpenMode::ReadWrite => GENERIC_READ | GENERIC_WRITE, FileOpenMode::Append => GENERIC_WRITE, FileOpenMode::Create => GENERIC_READ | GENERIC_WRITE, FileOpenMode::CreateNew => GENERIC_READ | GENERIC_WRITE, FileOpenMode::Truncate => GENERIC_WRITE };
    let share_flags = match share { FileShareMode::None => 0, FileShareMode::Read => FILE_SHARE_READ, FileShareMode::Write => FILE_SHARE_WRITE, FileShareMode::ReadWrite => FILE_SHARE_READ | FILE_SHARE_WRITE, FileShareMode::Delete => FILE_SHARE_DELETE };
    let disposition = match mode { FileOpenMode::Create => OPEN_ALWAYS, FileOpenMode::CreateNew => CREATE_NEW, FileOpenMode::Truncate => TRUNCATE_EXISTING, _ => OPEN_EXISTING };
    let h = unsafe { CreateFileW(w.as_ptr(), access, share_flags, std::ptr::null_mut(), disposition, FILE_ATTRIBUTE_NORMAL, std::ptr::null_mut()) };
    if h.is_null() { Err(()) } else { Ok(FileHandle(h)) }
}

pub fn close(h: FileHandle) -> Result<(), ()> { let ok = unsafe { CloseHandle(h.0) }; if ok == 0 { Err(()) } else { Ok(()) } }

pub fn read(h: FileHandle, buf: &mut [u8]) -> Result<usize, ()> {
    let mut n: u32 = 0;
    let ok = unsafe { ReadFile(h.0, buf.as_mut_ptr() as *mut c_void, buf.len() as u32, &mut n as *mut u32, std::ptr::null_mut()) };
    if ok == 0 { Err(()) } else { Ok(n as usize) }
}

pub fn write(h: FileHandle, buf: &[u8]) -> Result<usize, ()> {
    let mut n: u32 = 0;
    let ok = unsafe { WriteFile(h.0, buf.as_ptr() as *const c_void, buf.len() as u32, &mut n as *mut u32, std::ptr::null_mut()) };
    if ok == 0 { Err(()) } else { Ok(n as usize) }
}

pub fn size(h: FileHandle) -> Result<u64, ()> { let mut s: i64 = 0; let ok = unsafe { GetFileSizeEx(h.0, &mut s as *mut i64) }; if ok == 0 { Err(()) } else { Ok(s as u64) } }

pub fn seek(h: FileHandle, offset: i64, origin: SeekOrigin) -> Result<u64, ()> {
    let mut newpos: i64 = 0;
    let method = match origin { SeekOrigin::Begin => 0, SeekOrigin::Current => 1, SeekOrigin::End => 2 };
    let ok = unsafe { SetFilePointerEx(h.0, offset, &mut newpos as *mut i64, method) };
    if ok == 0 { Err(()) } else { Ok(newpos as u64) }
}

pub fn map(h: FileHandle, offset: u64, size: usize, access: MapAccess) -> Result<Mapping, ()> {
    let protect = match access { MapAccess::Read => PAGE_READONLY, MapAccess::Write | MapAccess::ReadWrite => PAGE_READWRITE };
    let map = unsafe { CreateFileMappingW(h.0, std::ptr::null_mut(), protect, 0, 0, std::ptr::null()) };
    if map.is_null() { return Err(()); }
    let desired = match access { MapAccess::Read => FILE_MAP_READ, MapAccess::Write => FILE_MAP_WRITE, MapAccess::ReadWrite => FILE_MAP_READ | FILE_MAP_WRITE };
    let addr = unsafe { MapViewOfFile(map, desired, (offset >> 32) as u32, (offset & 0xFFFF_FFFF) as u32, size) };
    if addr.is_null() { unsafe { CloseHandle(map) }; return Err(()); }
    Ok(Mapping { address: addr, length: size, native_mapping_handle: map })
}

pub fn unmap(m: &Mapping) -> Result<(), ()> { let ok = unsafe { UnmapViewOfFile(m.address) }; if ok == 0 { Err(()) } else { Ok(()) } }
pub fn flush_mapped(_h: FileHandle, addr: *mut c_void, size: usize) -> Result<(), ()> { let ok = unsafe { FlushViewOfFile(addr, size) }; if ok == 0 { Err(()) } else { Ok(()) } }

pub fn stdout_handle() -> FileHandle { FileHandle(unsafe { GetStdHandle(STD_OUTPUT_HANDLE) }) }
pub fn stderr_handle() -> FileHandle { FileHandle(unsafe { GetStdHandle(STD_ERROR_HANDLE) }) }

pub fn path_exists(path: &str) -> bool { unsafe { let w = wide(path); let a = GetFileAttributesW(w.as_ptr()); a != 0xFFFF_FFFF } }
pub fn path_is_directory(path: &str) -> bool { unsafe { let w = wide(path); let a = GetFileAttributesW(w.as_ptr()); a != 0xFFFF_FFFF && (a & FILE_ATTRIBUTE_DIRECTORY) != 0 } }
pub fn path_create_directory(path: &str) -> Result<(), ()> { let w = wide(path); let ok = unsafe { CreateDirectoryW(w.as_ptr(), std::ptr::null_mut()) }; if ok == 0 { Err(()) } else { Ok(()) } }
pub fn path_remove_file(path: &str) -> Result<(), ()> { let w = wide(path); let ok = unsafe { DeleteFileW(w.as_ptr()) }; if ok == 0 { Err(()) } else { Ok(()) } }
