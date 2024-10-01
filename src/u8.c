#include "u8.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unicase.h>
#include <unistr.h>

int u8_get(FILE* f, ucs4_t** ucs, size_t* s) {
  if (f == NULL) {
    errno = EXDEV;
    return -1;
  }

  size_t bs;

  if (f == stdin)
    bs = FMAX;
  else {
    if (fseek(f, 0, SEEK_END)) {
      errno = EIO;
      return -1;
    }

    if ((bs = ftell(f)) == -1) {
      errno = EIO;
      return -1;
    }

    if (fseek(f, 0, SEEK_SET)) {
      errno = EIO;
      return -1;
    }

    if (bs > FMAX) {
      errno = EFBIG;
      return -1;
    }

    if (bs == 0) {
      *s = 0;
      return 0;
    }
  }

  uint8_t* buf = malloc(bs + 1);
  bs = fread(buf, 1, bs, f);

  if (ferror(f)) {
    free(buf);
    errno = EIO;
    return -1;
  }

  ucs4_t* ubuf = malloc(bs * sizeof(ucs4_t));
  buf[bs] = 0;
  *s = 0;

  for (const uint8_t* iter = buf; (iter = u8_next(&ubuf[*s], iter)) != NULL;
       *s += 1) {
    ubuf[*s] = uc_tolower(ubuf[*s]);

    if (ubuf[*s] == 1105)
      ubuf[*s] -= 1;
  }

  ucs4_t* nubuf = realloc(ubuf, *s * sizeof(ucs4_t));

  if (nubuf == NULL) {
    free(ubuf);
    free(buf);
    errno = ENOMEM;
    return -1;
  }

  *ucs = nubuf;

  free(buf);

  return 0;
}

int u8_put(FILE* f, ucs4_t* ucs, size_t s) {
  if (f == NULL) {
    errno = EXDEV;
    return -1;
  }

  if (s == 0)
    return 0;

  uint8_t* buf = malloc(s * 4);
  size_t bs = 0;

  for (size_t i = 0; i < s; ++i) {
    if (ucs[i] == 1104)
      ucs[i] += 1;

    int r = u8_uctomb(buf + bs, ucs[i], 4);

    if (r == -1) {
      free(buf);
      errno = EPROTO;
      return -1;
    }

    bs += r;
  }

  if (fwrite(buf, 1, bs, f) < bs) {
    errno = EIO;
    free(buf);
    return -1;
  }

  free(buf);

  return 0;
}
