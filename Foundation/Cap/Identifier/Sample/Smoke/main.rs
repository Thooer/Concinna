use cap_identifier::*;
use cap_random::*;

fn main() {
    let mut r = Random::with_seed(987654321);
    let g = uuid_v4(&mut r);
    let s = uuid_to_string(&g);
    let id = string_id64("example");
    println!("uuid {} id64 {}", s, id);
}

