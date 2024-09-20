mod alph;
mod key;

fn main() {
    let alphabet = alph::Alphabet::new(33, 1072);
    let key = key::Key::gen(&alphabet);

    println!("{}", key.to_string(&alphabet));
}
