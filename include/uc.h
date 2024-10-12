#ifndef UC_H
#define UC_H

#include <stdio.h>
#include <unitypes.h>

#define FMAX UINT32_MAX

long u8_get(FILE* f, uint32_t n, ucs4_t* buf);
long u8_put(FILE* f, uint32_t n, ucs4_t* buf);

#endif  // UC_H
