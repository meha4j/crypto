#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unicase.h>

#include "enc.h"
#include "key.h"

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
    if (ucs[i] < A || ucs[i] > A + n)
      continue;

    buf[*rs] = key[ucs[i] - A] + A;
    *rs += 1;
  }

  buf = realloc(buf, *rs * sizeof(ucs4_t));
  *rucs = buf;

  return 0;
}

int enc_exe(int argc, char** argv) {
  return 0;
}

int dec_exe(int argc, char** argv) {
  return 0;
}
