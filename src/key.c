#include "key.h"
#include "u8.h"

#include <errno.h>
#include <sys/random.h>
#include <unistr.h>

static inline void swap(uint8_t* a, uint8_t* b) {
  if (*a == *b)
    return;

  uint8_t tmp = *a;
  *a = *b;
  *b = tmp;
}

int key_gen(uint8_t* key, uint8_t n) {
  for (int i = 0; i < n; ++i)
    key[i] = i;

  for (int i = 0; i < n - 1; ++i) {
    uint8_t max = UINT8_MAX;
    uint8_t rnd = UINT8_MAX;

    while (max % (n - i) != 0)
      max -= 1;

    while (rnd >= max)
      if (getrandom(&rnd, 1, 0) == -1)
        return -1;

    rnd = rnd % (n - i) + i;
    swap(&key[i], &key[rnd]);
  }

  return 0;
}

int key_get(FILE* f, uint8_t* key, uint8_t n, int inv) {
  ucs4_t* ucs;
  size_t s;

  if (u8_get(f, &ucs, &s)) {
    return -1;
  }

  if (s < n) {
    free(ucs);
    errno = EINVAL;
    return -1;
  }

  for (int i = 0; i < n; ++i) {
    if (ucs[i] < ORIGIN || ucs[i] > ORIGIN + n) {
      free(ucs);
      errno = EPROTO;
      return -1;
    }

    uint8_t off = ucs[i] - ORIGIN;

    if (inv)
      key[off] = i;
    else
      key[i] = off;
  }

  return 0;
}

int key_put(FILE* f, uint8_t* key, uint8_t n) {
  ucs4_t buf[n];

  for (int i = 0; i < n; ++i)
    buf[i] = ORIGIN + key[i];

  if (u8_put(f, buf, n)) {
    return -1;
  }

  return 0;
}
