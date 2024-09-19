use rand::{rngs::StdRng, RngCore, SeedableRng};

fn shuffle<T: Clone>(v: &mut [T]) {
    let mut rng = StdRng::from_entropy();

    for i in 0..v.len() - 1 {
        let rnd = rng.next_u32() as usize % (v.len() - i) + i;
        (v[i], v[rnd]) = (v[rnd].clone(), v[i].clone());
    }
}

fn main() {
    let mut key: Vec<u8> = (0..33).collect();

    shuffle(&mut key);

    unsafe {
        let _: Vec<u32> = key
            .iter()
            .map(|e| *e as u32 + 1072)
            .inspect(|e| print!("{}", char::from_u32_unchecked(*e)))
            .collect();
    }
}
