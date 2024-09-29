#ifndef U8_H
#define U8_H

#include <stdio.h>
#include <unitypes.h>

#define FMAX 0xffff

int u8_get(FILE* f, ucs4_t** ucs, size_t* s);
int u8_put(FILE* f, ucs4_t* ucs, size_t s);

#endif  // U8_H
