extern crate burro;

use burro::codec::CharReader;

fn main() {
    //    for i in 0..33u32 {
    //        println!(
    //            "{}: {}, {}",
    //            1072 + i,
    //            char::from_u32(1072 + i).unwrap(),
    //            char::from_u32(1072 + i - 32).unwrap()
    //        );
    //    }
    //
    let f = CharReader::open("in").unwrap();

    f.for_each(|u| println!("{}: {}", u, char::from_u32(u).unwrap()));
}
