use std::io::prelude::*;
use std::iter::Iterator;

pub struct Reader<T: Read> {
    src: T,
    buf: [u8; 1],
    loss: bool,
}

pub struct Writer<T: Write> {
    dst: T,
    buf: [u8; 4],
}

impl<T: Write> Writer<T> {
    pub fn create(dst: T) -> Self {
        Writer { dst, buf: [0u8; 4] }
    }

    pub fn write(&mut self, r: u32) -> Result<(), std::io::Error> {
        let s = char::from_u32(r).unwrap().encode_utf8(&mut self.buf).len();
        self.dst.write_all(&self.buf[0..s])
    }
}

impl<T: Read> Reader<T> {
    pub fn open(src: T) -> Self {
        Self {
            src,
            buf: [0u8; 1],
            loss: false,
        }
    }
}

impl<T: Read> Iterator for Reader<T> {
    type Item = u32;

    fn next(&mut self) -> Option<Self::Item> {
        if !self.loss {
            if let Err(error) = self.src.read_exact(&mut self.buf[0..1]) {
                return if let std::io::ErrorKind::UnexpectedEof = error.kind() {
                    None
                } else {
                    panic!("{error}")
                };
            }
        } else {
            self.loss = false;
        }

        let s = if self.buf[0] & 0x80 == 0x0 {
            0
        } else if self.buf[0] & 0xe0 == 0xc0 {
            1
        } else if self.buf[0] & 0xf0 == 0xe0 {
            2
        } else if self.buf[0] & 0xf8 == 0xf0 {
            3
        } else {
            return Some(char::REPLACEMENT_CHARACTER as u32);
        };

        let mut r = ((self.buf[0] & 0xff >> (s + 1)) as u32) << (s * 6);

        for i in 1..=s {
            self.src.read_exact(&mut self.buf[0..1]).unwrap();

            if self.buf[0] & 0xc0 == 0x80 {
                r |= ((self.buf[0] & 0x3f) as u32) << ((s - i) * 6);
            } else {
                self.loss = true;
                return Some(char::REPLACEMENT_CHARACTER as u32);
            }
        }

        Some(r)
    }
}
