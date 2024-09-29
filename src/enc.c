#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unicase.h>

#include "enc.h"

int enc(ucs4_t* ucs, size_t s, uint8_t* key, uint8_t n, ucs4_t** rucs,
        size_t* rs) {
  if (ucs == NULL || key == NULL) {
    errno = EXDEV;
    return -1;
  }

  if (n == 0) {
    errno = EPROTO;
    return -1;
  }

  *rs = 0;

  if (s == 0)
    return 0;

  ucs4_t* buf = malloc(s * sizeof(ucs4_t));

  for (size_t i = 0; i < s; ++i) {
    ucs4_t uc = uc_tolower(ucs[i]);

    if (uc < ORIGIN || uc > ORIGIN + n)
      continue;

    if (uc == 1105)
      uc--;

    buf[*rs] = key[uc - ORIGIN] + ORIGIN;
    *rs += 1;
  }

  buf = realloc(buf, *rs * sizeof(ucs4_t));
  *rucs = buf;

  return 0;
}
