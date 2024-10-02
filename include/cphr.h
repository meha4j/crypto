#ifndef CPHR_H
#define CPHR_H

#include <stdint.h>
#include <unitypes.h>

#define N 33
#define A 1072

typedef uint8_t key[N];

int key_gen(key k);

int key_touc(key k, ucs4_t uc[N]);
int key_fruc(key k, ucs4_t uc[N], int inv);

int encode(size_t ds, ucs4_t data[ds], key k);

#endif  // CPHR_H
