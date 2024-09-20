use crate::alph::*;
use rand::{rngs::StdRng, RngCore, SeedableRng};

pub struct Key {
    pub offset: Vec<u32>,
}

impl Key {
    pub fn gen(a: &Alphabet) -> Self {
        Key {
            offset: permutation(a.size()),
        }
    }

    pub fn to_string(&self, a: &Alphabet) -> String {
        self.offset
            .iter()
            .map(|o| char::from_u32(a.origin() + o).unwrap())
            .collect()
    }
}

fn permutation(size: u32) -> Vec<u32> {
    let mut key: Vec<u32> = (0..size).collect();

    shuffle(&mut key);

    key
}

fn shuffle<T: Clone>(v: &mut [T]) {
    let mut rng = StdRng::from_entropy();

    for i in 0..v.len() - 1 {
        let mut max = usize::MAX;
        let mut rnd = rng.next_u64() as usize;

        while max % (v.len() - i) != 0 {
            max = max - 1;
        }

        while rnd >= max {
            rnd = rng.next_u64() as usize;
        }

        rnd = rnd % (v.len() - i) + i;
        (v[i], v[rnd]) = (v[rnd].clone(), v[i].clone());
    }
}
