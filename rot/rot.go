package rot

import (
	"fmt"
	"io"
	"strings"
	"unicode"
	"unicode/utf8"
)

type PrivateKey map[rune]rune

type PublicKey map[rune]rune

type Encrypter struct {
	src io.RuneReader
	key *PrivateKey
}

type Decrypter struct {
	src io.RuneReader
	key *PublicKey
}

func NewEncrypter(src io.RuneReader, key *PrivateKey) Encrypter {
	return Encrypter{src, key}
}

func NewDecrypter(src io.RuneReader, key *PublicKey) Decrypter {
	return Decrypter{src, key}
}

func GenerateKey(rand io.Reader) (*PrivateKey, error) {
	var rnd [33]byte

	_, err := rand.Read(rnd[:])

	if err != nil {
		return nil, err
	}

	var key PrivateKey = make(map[rune]rune, 34)

	for i := 1072; i < 1105; i++ {
		key[rune(i)] = rune(i)
	}

	for i := 32; i > 0; i-- {
		rnd[i] = uint8(float64(rnd[i]) / 0xff.p0 * float64(i))

		a := rune(i) + 1072
		b := rune(rnd[i]) + 1072

		if key[b] == 1104 {
			key[b] = 1105
		}

		key[a], key[b] = key[b], key[a]
	}

	key[1105] = key[1104]
	delete(key, 1104)

	return &key, nil
}

func ReadKey(src io.RuneReader) (*PrivateKey, error) {
	var key PrivateKey = make(map[rune]rune, 33)

	for i := 1072; i < 1105; i++ {
		r, _, err := src.ReadRune()

		if err != nil {
			return nil, err
		}

		if r < 1072 || r > 1105 || r == 1104 {
			return nil, fmt.Errorf("Malformed key.")
		}

		if i == 1104 {
			i = 1105
		}

		key[rune(i)] = r
	}

	return &key, nil
}

func (key *PrivateKey) String() string {
	var ord [33]rune

	for k, v := range *key {
		if k == 1105 {
			k = 1078
		} else if k > 1077 {
			k += 1
		}

		ord[k-1072] = v
	}

	var sb strings.Builder

	for _, v := range ord {
		sb.WriteRune(v)
	}

	return sb.String()
}

func (key *PrivateKey) Public() *PublicKey {
	var pkey PublicKey = make(map[rune]rune, 33)

	for k, v := range *key {
		pkey[v] = k
	}

	return &pkey
}

func (self *Encrypter) Read(b []byte) (s int, err error) {
	for {
		r, _, err := self.ReadRune()

		if err != nil {
			return s, err
		}

		if utf8.RuneLen(r) > len(b) {
			return s, nil
		}

		l := utf8.EncodeRune(b, r)

		b = b[l:]
		s = s + l
	}
}

func (self *Encrypter) ReadRune() (rune, int, error) {
	for {
		r, _, err := self.src.ReadRune()

		if err != nil {
			return utf8.RuneError, 0, err
		}

    r = unicode.ToLower(r)
		r, ok := (*self.key)[r]

		if !ok {
			continue
		}

		return r, utf8.RuneLen(r), nil
	}
}

func (self *Decrypter) Read(b []byte) (s int, err error) {
	for {
		r, _, err := self.ReadRune()

		if err != nil {
			return s, err
		}

		if utf8.RuneLen(r) > len(b) {
			return s, nil
		}

		l := utf8.EncodeRune(b, r)

		b = b[l:]
		s = s + l
	}
}

func (self *Decrypter) ReadRune() (rune, int, error) {
	for {
		r, _, err := self.src.ReadRune()

		if err != nil {
			return utf8.RuneError, 0, err
		}

		r, ok := (*self.key)[r]

		if !ok {
			continue
		}

		return r, utf8.RuneLen(r), nil
	}
}
