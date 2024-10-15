#include <uc.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unicase.h>
#include <unistr.h>

long u8_get(FILE* f, uint32_t n, ucs4_t* buf) {
  if (f == NULL) {
    errno = EINVAL;
    return -1;
  }

  long bs = n * 4;

  if (n == 0) {
    if (f == stdin)
      bs = FMAX;
    else {
      if (fseek(f, 0, SEEK_END))
        return -1;

      if ((bs = ftell(f)) == -1L)
        return -1;

      if (fseek(f, 0, SEEK_SET))
        return -1;

      if (bs > FMAX) {
        errno = EFBIG;
        return -1;
      }
    }
  }

  uint8_t* buff = malloc(bs + 1);

  if (!buff) {
    errno = ENOMEM;
    return -1;
  }

  bs = fread(buff, 1, bs, f);

  if (ferror(f)) {
    free(buff);
    return -1;
  }

  if (bs == 0) {
    free(buff);
    return 0;
  }

  buff[bs] = 0;
  long s = 0;

  const uint8_t* iter = buff;

  while ((iter = u8_next(&buf[s], iter))) {
    buf[s] = uc_tolower(buf[s]);

    if (buf[s] == 1105)
      buf[s] -= 1;
    else if (buf[s] == 1104)
      buf[s] += 1;

    s += 1;
  }

  free(buff);
  return s;
}

long u8_put(FILE* f, uint32_t n, ucs4_t* buf) {
  if (f == NULL) {
    errno = EXDEV;
    return -1;
  }

  if (n == 0)
    return 0;

  uint8_t* buff = malloc(n * 4);
  uint64_t s = 0;

  for (size_t i = 0; i < n; ++i) {
    if (buf[i] == 1104)
      buf[i] += 1;

    int r = u8_uctomb(buff + s, buf[i], 4);

    if (r == -1) {
      free(buff);
      errno = EPROTO;
      return -1;
    }

    s += r;
  }

  if (fwrite(buff, 1, s, f) < s) {
    free(buff);
    return -1;
  }

  free(buff);
  return n;
}
