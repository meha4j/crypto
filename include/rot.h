#ifndef ROT_H
#define ROT_H

#include <stdbool.h>
#include <stdint.h>
#include <unitypes.h>

#define KS 33
#define A 1072

int kgen(uint8_t* k);
int kinv(uint8_t* k);

int k2uc(uint8_t* k, ucs4_t* uc);
int uc2k(uint8_t* k, ucs4_t* uc);

long rot(uint32_t ds, ucs4_t data[ds], uint8_t* k);

#endif  // ROT_H
