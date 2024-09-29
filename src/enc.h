#ifndef ENC_H
#define ENC_H

#include <unitypes.h>

int enc(ucs4_t* ucs, size_t s, uint8_t* key, uint8_t n, ucs4_t** rucs,
        size_t* rs);

#endif  // ENC_H
