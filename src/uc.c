#include <uc.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unicase.h>
#include <unistr.h>

int u8_get(FILE* f, size_t bs, ucs4_t* buf) {
  if (f == NULL) {
    errno = EINVAL;
    return -1;
  }

  int size = bs * 4;

  if (bs == 0) {
    if (f == stdin)
      size = FMAX;
    else {
      if (fseek(f, 0, SEEK_END)) {
        return -1;
      }

      if ((size = ftell(f)) == -1L) {
        return -1;
      }

      if (fseek(f, 0, SEEK_SET)) {
        return -1;
      }

      if (size > FMAX) {
        errno = EFBIG;
        return -1;
      }
    }
  }

  uint8_t* buff = malloc(size + 1);

  if (!buff) {
    errno = ENOMEM;
    return -1;
  }

  size = fread(buff, 1, size, f);

  if (ferror(f)) {
    free(buff);
    errno = EIO;
    return -1;
  }

  buff[size] = 0;
  int s = 0;

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

int u8_put(FILE* f, size_t bs, ucs4_t* buf) {
  if (f == NULL) {
    errno = EXDEV;
    return -1;
  }

  if (bs == 0)
    return 0;

  uint8_t* buff = malloc(bs * 4);
  size_t s = 0;

  for (size_t i = 0; i < bs; ++i) {
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
    errno = EIO;
    free(buff);
    return -1;
  }

  free(buff);
  return bs;
}
