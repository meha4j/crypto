pub struct Alphabet {
    size: u32,
    origin: u32,
}

impl Alphabet {
    pub fn new(size: u32, origin: u32) -> Self {
        if size == 0 {
            panic!("Alphabet must contain at least one letter.");
        }

        Alphabet { size, origin }
    }

    pub fn size(&self) -> u32 {
        self.size
    }

    pub fn origin(&self) -> u32 {
        self.origin
    }
}
