use std::fs::File;
use std::io::Read;
use std::iter::Iterator;

pub struct CharReader {
    file: File,
}

impl CharReader {
    pub fn open(f: &str) -> Result<Self, std::io::Error> {
        Ok(CharReader {
            file: File::open(f)?,
        })
    }
}

impl Iterator for CharReader {
    type Item = u32;

    fn next(&mut self) -> Option<Self::Item> {
        let mut v = [0u8, 4];

        if let Err(error) = self.file.read_exact(&mut v[0..1]) {
            match error.kind() {
                std::io::ErrorKind::UnexpectedEof => return None,
                _ => panic!(),
            }
        }

        let s = if v[0] < 128u8 {
            0
        } else if v[0] < 224u8 {
            1
        } else if v[0] < 240u8 {
            2
        } else {
            3
        };

        self.file.read_exact(&mut v[1..=s]).unwrap();

        let mut r = ((v[0] & 0b11111111u8 >> (s + 1)) as u32) << (s * 6);

        for i in 1..=s {
            r |= ((v[i] & 0b00111111u8) as u32) << ((s - i) * 6);
        }

        Some(r)
    }
}
