// burro key [options] keyfile
//
// burro enc [options] keyfile
//  -in infile      File to encode - default stdin
//  -out outfile    Output file - default stdout
//
// burro dec [options] keyfile
//  -in infile      File to decode - default stdin
//  -out outfile    Output file - default stdout
//
// burro crk [options]
//  -in infile      File to decode - default stdin
//  -out outfile    Output file - default stdout
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <unistd.h>
#include <unistr.h>
#include <unitypes.h>

#define ALPH 33
#define US 2
#define UC(off) ((ucs4_t)1072 + (ucs4_t)(off))

static inline void swap(uint8_t* a, uint8_t* b) {
  uint8_t tmp = *a;
  *a = *b;
  *b = tmp;
}

void key_gen(uint8_t* key) {
  for (size_t i = 0; i < ALPH; ++i)
    key[i] = i;

  for (size_t i = 0; i < ALPH - 1; ++i) {
    uint8_t max = UINT8_MAX;
    uint8_t rnd = UINT8_MAX;

    while (max % (ALPH - i) != 0)
      max -= 1;

    while (rnd >= max)
      getrandom(&rnd, 1, 0);

    rnd = rnd % (ALPH - i) + i;
    swap(&key[i], &key[rnd]);
  }
}

void key_get();

void key_put(FILE* f, uint8_t* key) {
  uint8_t* buf = malloc(ALPH * US);

  for (size_t i = 0; i < ALPH; ++i)
    u8_uctomb(buf + i * US, UC(key[i]), US);

  fwrite(buf, 1, ALPH * US, f);
}

void key_inv();

void enc(FILE* f);

void dec();

int main() {

  return 0;
}
