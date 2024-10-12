#include <rot.h>
#include <uc.h>

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/random.h>

static inline void swap(uint8_t* a, uint8_t* b) {
  if (*a == *b)
    return;

  uint8_t tmp = *a;
  *a = *b;
  *b = tmp;
}

int kgen(uint8_t* k) {
  for (int i = 0; i < KS; ++i)
    k[i] = i;

  for (int i = 0; i < KS - 1; ++i) {
    uint8_t max = UINT8_MAX;
    uint8_t rnd = UINT8_MAX;

    while (max % (KS - i) != 0)
      max -= 1;

    while (rnd >= max)
      if (getrandom(&rnd, 1, 0) == -1) {
        errno = EIO;
        return -1;
      }

    rnd = rnd % (KS - i) + i;
    swap(&k[i], &k[rnd]);
  }

  return 0;
}

int kinv(uint8_t* k) {
  uint8_t kr[KS];

  for (int i = 0; i < KS; ++i)
    kr[k[i]] = i;

  memcpy(k, kr, KS);

  return 0;
}

int k2uc(uint8_t* k, ucs4_t* uc) {
  for (int i = 0; i < KS; ++i) {
    int j = i;

    if (i == 6)
      j = 32;
    else if (i > 6)
      j = i - 1;

    uc[i] = A + k[j];
  }

  return 0;
}

int uc2k(uint8_t* k, ucs4_t* uc) {
  uint64_t f = 0;

  for (int i = 0; i < KS; ++i) {
    if (uc[i] < A || uc[i] >= A + KS) {
      errno = EINVAL;
      return -1;
    }

    int j = i;

    if (i == 6)
      j = 32;
    else if (i > 6)
      j = i - 1;

    uint8_t off = uc[i] - A;

    if (f & (1UL << off)) {
      errno = EINVAL;
      return -1;
    }

    f |= (1UL << off);
    k[j] = off;
  }

  return 0;
}

long rot(uint32_t ds, ucs4_t data[ds], uint8_t* k) {
  long s = 0;

  for (int i = 0; i < ds; ++i)
    if (data[i] >= A && data[i] < A + KS)
      data[s++] = A + k[data[i] - A];

  return s;
}
