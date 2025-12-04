#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PathError { Failed, BufferTooSmall, InvalidArgument }

fn is_sep(b: u8) -> bool { b == b'/' || b == b'\\' }

pub fn to_unix(src: &str, out: &mut [u8]) -> Result<usize, PathError> {
    let bs = src.as_bytes();
    if out.len() < bs.len() { return Err(PathError::BufferTooSmall); }
    for i in 0..bs.len() { out[i] = if bs[i] == b'\\' { b'/' } else { bs[i] }; }
    Ok(bs.len())
}

pub fn to_windows(src: &str, out: &mut [u8]) -> Result<usize, PathError> {
    let bs = src.as_bytes();
    if out.len() < bs.len() { return Err(PathError::BufferTooSmall); }
    for i in 0..bs.len() { out[i] = if bs[i] == b'/' { b'\\' } else { bs[i] }; }
    Ok(bs.len())
}

pub fn normalize(src: &str, out: &mut [u8]) -> Result<usize, PathError> {
    let bs = src.as_bytes();
    if out.len() < bs.len() { return Err(PathError::BufferTooSmall); }
    let mut o = 0usize;
    let mut prev_sep = false;
    for &c in bs.iter() {
        if is_sep(c) {
            if !prev_sep { out[o] = b'/'; o += 1; }
            prev_sep = true;
        } else {
            out[o] = c; o += 1; prev_sep = false;
        }
    }
    if o > 1 && out[o-1] == b'/' { o -= 1; }
    Ok(o)
}

pub fn join(a: &str, b: &str, out: &mut [u8]) -> Result<usize, PathError> {
    let ab = a.as_bytes();
    let bb = b.as_bytes();
    let mut alen = ab.len();
    while alen > 0 && is_sep(ab[alen-1]) { alen -= 1; }
    let mut boff = 0usize;
    while boff < bb.len() && is_sep(bb[boff]) { boff += 1; }
    let need = alen.checked_add(1).and_then(|v| v.checked_add(bb.len()-boff)).ok_or(PathError::Failed)?;
    if out.len() < need { return Err(PathError::BufferTooSmall); }
    for i in 0..alen { out[i] = ab[i]; }
    out[alen] = b'/';
    let mut o = alen + 1;
    for i in boff..bb.len() { out[o] = bb[i]; o += 1; }
    Ok(o)
}

pub fn split_ext(name: &str) -> (usize, usize) {
    let nb = name.as_bytes();
    let mut last_sep = None;
    let mut last_dot = None;
    for i in 0..nb.len() {
        let c = nb[i];
        if is_sep(c) { last_sep = Some(i); last_dot = None; } else if c == b'.' { last_dot = Some(i); }
    }
    match last_dot {
        Some(d) => { let base = d; let ext = nb.len().saturating_sub(d+1); (base, ext) }
        None => (nb.len(), 0)
    }
}

