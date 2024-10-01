#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unicase.h>

#include "enc.h"
#include "key.h"
#include "u8.h"

static int enc(size_t rs, ucs4_t* raw, uint8_t* key, size_t* os, ucs4_t** out) {
  if (raw == NULL || key == NULL) {
    errno = EXDEV;
    return -1;
  }

  *os = 0;

  if (rs == 0)
    return 0;

  ucs4_t* buf = malloc(rs * sizeof(ucs4_t));

  for (size_t i = 0; i < rs; ++i) {
    if (raw[i] < A || raw[i] >= A + N)
      continue;

    buf[*os] = key[raw[i] - A] + A;
    *os += 1;
  }

  buf = realloc(buf, *os * sizeof(ucs4_t));
  *out = buf;

  return 0;
}

static int help(int inv) {
  printf("Usage: %s [OPTIONS] KEYFILE\n\n", inv ? "dec" : "enc");
  printf("KEYFILE\t\tFile with private key.\n\n");
  printf("Available options:\n");
  printf("\t-in INFILE\t\tFile to %scode, default: stdin.\n",
         inv ? "de" : "en");
  printf("\t-out OUTFILE\t\tOutput file, default: stdout.\n");

  return 0;
}

int exe(int argc, char* argv[argc], int inv) {
  if (argc < 3 || !strcmp(argv[2], "help"))
    return help(inv);

  FILE* f = 0;
  FILE* in = stdin;
  FILE* out = stdout;

  for (int i = 2; i < argc; ++i) {
    if (!strcmp(argv[i], "-in")) {
      if (i + 1 == argc) {
        errno = EINVAL;
        goto err;
      }

      in = fopen(argv[++i], "r");

      if (!in) {
        errno = EIO;
        goto err;
      }
    } else if (!strcmp(argv[i], "-out")) {
      if (i + 1 == argc) {
        errno = EINVAL;
        goto err;
      }

      out = fopen(argv[++i], "w+");

      if (!out) {
        errno = EIO;
        goto err;
      }
    } else {
      f = fopen(argv[i], "r");

      if (!f) {
        errno = EIO;
        goto err;
      }

      uint8_t key[N];

      ucs4_t* raw;
      ucs4_t* mut;

      size_t rs;
      size_t ms;

      if (key_get(f, key, inv))
        goto err;

      if (u8_get(in, &raw, &rs))
        goto err;

      if (enc(rs, raw, key, &ms, &mut))
        goto err;

      if (u8_put(out, mut, ms))
        goto err;

      goto end;
    }
  }

  errno = EPROTO;

err:
  if (f)
    fclose(f);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  return -1;

end:
  if (f)
    fclose(f);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  return 0;
}

int enc_exe(int argc, char* argv[argc]) {
  return exe(argc, argv, 0);
}

int dec_exe(int argc, char* argv[argc]) {
  return exe(argc, argv, 1);
}
