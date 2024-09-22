extern crate burro;

use burro::key::*;
use burro::utf8::*;

use std::fs::File;

fn main() {
    let key = Key::get(File::open("ru.key").unwrap(), 33, 1072);
    let inp = Reader::open(File::open("in.burro").unwrap());
    let mut out = Writer::create(File::create("in.burro.burro").unwrap());

    inp
        .filter(|u| *u >= 1072)
        .map(|u| u - 1072)
        .map(|off| key.at(off) as u32 + 1072)
        .for_each(|u| out.write(u).unwrap());
}
