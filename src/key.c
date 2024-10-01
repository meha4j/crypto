#include "key.h"
#include "u8.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/random.h>
#include <unistr.h>

static inline void swap(uint8_t* a, uint8_t* b) {
  if (*a == *b)
    return;

  uint8_t tmp = *a;
  *a = *b;
  *b = tmp;
}

int key_gen(uint8_t* key) {
  for (int i = 0; i < N; ++i)
    key[i] = i;

  for (int i = 0; i < N - 1; ++i) {
    uint8_t max = UINT8_MAX;
    uint8_t rnd = UINT8_MAX;

    while (max % (N - i) != 0)
      max -= 1;

    while (rnd >= max)
      if (getrandom(&rnd, 1, 0) == -1)
        return -1;

    rnd = rnd % (N - i) + i;
    swap(&key[i], &key[rnd]);
  }

  return 0;
}

int key_get(FILE* f, uint8_t* key, int inv) {
  ucs4_t* ucs;
  size_t s;

  if (u8_get(f, &ucs, &s)) {
    return -1;
  }

  if (s < N) {
    free(ucs);
    errno = EINVAL;
    return -1;
  }

  for (int i = 0; i < N; ++i) {
    if (ucs[i] < A || ucs[i] > A + N) {
      free(ucs);
      errno = EPROTO;
      return -1;
    }

    uint8_t off = ucs[i] - A;

    if (inv)
      key[off] = i;
    else
      key[i] = off;
  }

  return 0;
}

int key_put(FILE* f, uint8_t* key) {
  ucs4_t buf[N];

  for (int i = 0; i < N; ++i)
    buf[i] = A + key[i];

  if (u8_put(f, buf, N)) {
    return -1;
  }

  return 0;
}

static int help() {
  printf("Usage: key KEYFILE\n\n");
  printf("KEYFILE\t\tOutput file.\n");

  return 0;
}

int key_exe(int argc, char* argv[argc]) {
  if (argc < 3 || !strcmp(argv[2], "help"))
    return help();

  FILE* f = fopen(argv[2], "w+");

  if (!f) {
    errno = EIO;
    return -1;
  }

  uint8_t key[N];

  if (key_gen(key)) {
    fclose(f);
    return -1;
  }

  if (key_put(f, key)) {
    fclose(f);
    return -1;
  }

  fclose(f);

  return 0;
}
