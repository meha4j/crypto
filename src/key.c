// burro key KEYFILE

#include "key.h"

#include <string.h>
#include <sys/random.h>
#include <unistr.h>

static inline void swap(uint8_t* a, uint8_t* b) {
  uint8_t tmp = *a;
  *a = *b;
  *b = tmp;
}

int key_gen(uint8_t* key) {
  for (int i = 0; i < ALPH_SIZE; ++i)
    key[i] = i;

  for (int i = 0; i < ALPH_SIZE - 1; ++i) {
    uint8_t max = UINT8_MAX;
    uint8_t rnd = UINT8_MAX;

    while (max % (ALPH_SIZE - i) != 0)
      max -= 1;

    while (rnd >= max)
      if (getrandom(&rnd, 1, 0) == -1)
        return -1;

    rnd = rnd % (ALPH_SIZE - i) + i;
    swap(&key[i], &key[rnd]);
  }

  return 0;
}

int key_get(FILE* f, uint8_t key[ALPH_SIZE], bool inv) {
  uint8_t buf[KEY_LEN];
  ucs4_t uc;

  if (fread(buf, 1, KEY_LEN, f) < KEY_LEN)
    return -1;

  const uint8_t* iter = buf;

  for (int i = 0; i < ALPH_SIZE; ++i) {
    iter = u8_next(&uc, iter);

    if (uc < ALPH_BEG || uc > ALPH_BEG + ALPH_SIZE)
      return -1;

    int off = uc - ALPH_BEG;

    if (inv)
      key[off] = i;
    else
      key[i] = off;
  }

  return 0;
}

int key_put(FILE* f, uint8_t* key) {
  uint8_t buf[KEY_LEN];
  size_t size = 0;

  for (int i = 0; i < ALPH_SIZE; ++i)
    size += u8_uctomb(buf + size, ALPH_BEG + key[i], 4);

  if (fwrite(buf, 1, size, f) != size)
    return -1;

  return 0;
}

static int help() {
  printf("Usage: key KEYFILE\n\n");
  printf("KEYFILE\t\tOutput file.\n");

  return 0;
}

int key_exe(int argc, char* argv[argc]) {
  if (argc != 3 || (strcmp(argv[2], "help") == 0))
    return help();

  FILE* out = fopen(argv[2], "w+");

  if (!out) {
    printf("key: i/o error.\n");
    return -1;
  }

  uint8_t key[ALPH_SIZE];

  if (key_gen(key)) {
    printf("key: generation error.\n");
    goto ferr;
  }

  if (key_put(out, key)) {
    printf("key: i/o error.\n");
    goto ferr;
  }

  printf("✓ Completed\n");

  fclose(out);
  return 0;

ferr:
  fclose(out);
  return -1;
}
