use crate::utf8::*;

use rand::prelude::*;
use std::io::prelude::*;

pub struct Key {
    val: Vec<u8>,
    bgn: u32,
}

impl Key {
    pub fn gen(size: u8, bgn: u32) -> Self {
        let mut key = Key {
            val: (0..size).collect(),
            bgn,
        };

        key.shuffle();
        key
    }

    pub fn get<T: Read>(f: T, size: u32, bgn: u32) -> Self {
        let val: Vec<u8> = Reader::open(f)
            .take(size as usize)
            .map(|u| (u - bgn) as u8)
            .collect();

        Key { val, bgn }
    }

    pub fn put<T: Write>(&self, f: T) -> Result<(), std::io::Error> {
        let mut out = Writer::create(f);

        for off in self.val.iter() {
            out.write(self.bgn + *off as u32)?;
        }

        Ok(())
    }

    pub fn at(&self, off: u32) -> u8 {
        self.val[off as usize]
    }

    pub fn len(&self) -> u32 {
        self.val.len() as u32
    }

    fn shuffle(&mut self) {
        let mut rng = StdRng::from_entropy();

        for i in 0..self.val.len() - 1 {
            let mut max = usize::MAX;
            let mut rnd = rng.next_u64() as usize;

            while max % (self.val.len() - i) != 0 {
                max = max - 1;
            }

            while rnd >= max {
                rnd = rng.next_u64() as usize;
            }

            rnd = rnd % (self.val.len() - i) + i;
            (self.val[i], self.val[rnd]) = (self.val[rnd].clone(), self.val[i].clone());
        }
    }
}
