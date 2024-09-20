use rand::prelude::*;
use std::fs::File;
use std::io::prelude::*;
use std::string::ToString;

pub struct Key {
    value: Vec<u32>,
    origin: u32,
}

impl Key {
    pub fn gen(size: u32, origin: u32) -> Self {
        let mut key = Key {
            value: (0..size).collect(),
            origin,
        };

        key.shuffle();
        key
    }

    pub fn get(f: &str, size: u32, origin: u32) -> Result<Self, std::io::Error> {
        let mut key = String::new();

        File::open(f)?.read_to_string(&mut key)?;

        Ok(Key {
            value: key
                .chars()
                .take(size as usize)
                .map(|c| c as u32 - origin)
                .collect(),
            origin,
        })
    }

    pub fn put(&self, f: &str) -> Result<(), std::io::Error> {
        write!(File::create(f)?, "{}", self.to_string())
    }

    pub fn at(&self, off: u32) -> u32 {
        self.value[off as usize]
    }

    pub fn len(&self) -> u32 {
        self.value.len() as u32
    }

    fn shuffle(&mut self) {
        let mut rng = StdRng::from_entropy();

        for i in 0..self.value.len() - 1 {
            let mut max = usize::MAX;
            let mut rnd = rng.next_u64() as usize;

            while max % (self.value.len() - i) != 0 {
                max = max - 1;
            }

            while rnd >= max {
                rnd = rng.next_u64() as usize;
            }

            rnd = rnd % (self.value.len() - i) + i;
            (self.value[i], self.value[rnd]) = (self.value[rnd].clone(), self.value[i].clone());
        }
    }
}

impl ToString for Key {
    fn to_string(&self) -> String {
        self.value
            .iter()
            .map(|o| char::from_u32(self.origin + o).unwrap())
            .collect()
    }
}
