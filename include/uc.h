#ifndef UC_H
#define UC_H

#include <stdio.h>
#include <unitypes.h>

#define FMAX 0xffffff

int u8_get(FILE* f, size_t bs, ucs4_t* buf);
int u8_put(FILE* f, size_t bs, ucs4_t* buf);

#endif  // UC_H
