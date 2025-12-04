use cap_stream::*;

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum ConfigError { Failed }

#[derive(Clone, Debug)]
pub struct Entry { pub section: Option<String>, pub key: String, pub value: String }

#[derive(Clone, Debug)]
pub struct Config { pub entries: Vec<Entry> }

impl Config {
    pub fn load(path: &str) -> Result<Self, ConfigError> {
        let mut r = FileReader::open(path).map_err(|_| ConfigError::Failed)?;
        let sz = r.size().map_err(|_| ConfigError::Failed)? as usize;
        let mut buf = vec![0u8; sz];
        let _ = r.read(&mut buf).map_err(|_| ConfigError::Failed)?;
        let s = String::from_utf8_lossy(&buf);
        Ok(Self { entries: parse_ini(&s) })
    }
    pub fn get(&self, section: Option<&str>, key: &str) -> Option<&str> {
        for e in &self.entries { if e.key == key && match (section.as_deref(), e.section.as_deref()) { (None, None) => true, (Some(a), Some(b)) => a==b, (None, Some(_)) => false, (Some(_), None) => false } { return Some(&e.value); } }
        None
    }
    pub fn get_str(&self, section: Option<&str>, key: &str) -> Option<String> { self.get(section, key).map(|v| v.to_string()) }
    pub fn get_u32(&self, section: Option<&str>, key: &str) -> Option<u32> { self.get(section, key).and_then(|v| v.parse::<u32>().ok()) }
    pub fn get_f32(&self, section: Option<&str>, key: &str) -> Option<f32> { self.get(section, key).and_then(|v| v.parse::<f32>().ok()) }
}

fn parse_ini(s: &str) -> Vec<Entry> {
    let mut cur: Option<String> = None; let mut out = Vec::<Entry>::new();
    for line in s.lines() {
        let t = line.trim(); if t.is_empty() { continue; }
        if t.starts_with('#') || t.starts_with(';') { continue; }
        if t.starts_with('[') && t.ends_with(']') { cur = Some(t[1..t.len()-1].trim().to_string()); continue; }
        let (k, v) = if let Some(i) = t.find('=') { (&t[..i], &t[i+1..]) } else if let Some(j) = t.find(':') { (&t[..j], &t[j+1..]) } else { (t, "") };
        out.push(Entry { section: cur.clone(), key: k.trim().to_string(), value: v.trim().to_string() });
    }
    out
}

