package rot

import (
	"crypto/rand"
)

const A = 1072
const S = 33

func kgen(k []byte) error {
	for i := range k {
		k[i] = byte(i)
	}

	var rnd [S - 1]byte

	_, err := rand.Read(rnd[:])

	if err != nil {
		return err
	}

	for i := S - 1; i > 0; i-- {
		rnd[i] = uint8(float64(rnd[i]) / 0xff.p0 * float64(i))
		k[rnd[i]], k[i] = k[i], k[rnd[i]]
	}

	return nil
}

func rot(k []byte, d []rune) (int, error) {
	j := 0

	for i, r := range d {
		if r < A || r >= A+S {
			continue
		}

		d[j] = rune(uint32(k[d[i]-A]) + A)
		j += 1
	}

	return j, nil
}
