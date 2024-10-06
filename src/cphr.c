#include <cphr.h>
#include <uc.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>

static inline void swap(uint8_t* a, uint8_t* b) {
  if (*a == *b)
    return;

  uint8_t tmp = *a;
  *a = *b;
  *b = tmp;
}

int key_gen(key k) {
  for (int i = 0; i < N; ++i)
    k[i] = i;

  for (int i = 0; i < N - 1; ++i) {
    uint8_t max = UINT8_MAX;
    uint8_t rnd = UINT8_MAX;

    while (max % (N - i) != 0)
      max -= 1;

    while (rnd >= max)
      if (getrandom(&rnd, 1, 0) == -1) {
        errno = EIO;
        return -1;
      }

    rnd = rnd % (N - i) + i;
    swap(&k[i], &k[rnd]);
  }

  return 0;
}

int key_touc(key k, ucs4_t uc[N]) {
  for (int i = 0; i < N; ++i) {
    int j = i;

    if (i == 6)
      j = 32;
    else if (i > 6)
      j = i - 1;

    uc[i] = A + k[j];
  }

  return 0;
}

int key_fruc(key k, ucs4_t uc[N], int inv) {
  uint64_t f = 0;

  for (int i = 0; i < N; ++i) {
    if (uc[i] < A || uc[i] >= A + N) {
      errno = EINVAL;
      return -1;
    }

    int j = i;

    if (i == 6)
      j = 32;
    else if (i > 6)
      j = i - 1;

    uint8_t off = uc[i] - A;

    if (f & (1UL << off)) {
      errno = EINVAL;
      return -1;
    }

    f |= (1UL << off);

    if (inv)
      k[off] = j;
    else
      k[j] = off;
  }

  return 0;
}

int encode(size_t ds, ucs4_t data[ds], key k) {
  int s = 0;

  for (int i = 0; i < ds; ++i)
    if (data[i] >= A && data[i] < A + N)
      data[s++] = A + k[data[i] - A];

  return s;
}

static int key_help() {
  printf("Usage: key KEYFILE\n\n");
  printf("KEYFILE\t\tOutput file.\n");

  return 0;
}

int key_exe(int argc, char* argv[argc]) {
  if (argc < 3 || !strcmp(argv[2], "help"))
    return key_help();

  FILE* f = fopen(argv[2], "w+");

  if (!f) {
    errno = EIO;
    goto err;
  }

  ucs4_t data[N];
  key k;

  if (key_gen(k))
    goto err;

  if (key_touc(k, data))
    goto err;

  if (u8_put(f, N, data) != N)
    goto err;

  fclose(f);
  return 0;

err:
  if (f)
    fclose(f);

  return -1;
}

static int help(int inv) {
  printf("Usage: %s [OPTIONS] KEYFILE\n\n", inv ? "dec" : "enc");
  printf("KEYFILE\t\tFile with private key.\n\n");
  printf("Available options:\n");
  printf("\t-i INFILE\t\tFile to %scode, default: stdin.\n", inv ? "de" : "en");
  printf("\t-o OUTFILE\t\tOutput file, default: stdout.\n");

  return 0;
}

static int exe(int argc, char* argv[argc], int inv) {
  if (argc < 3 || !strcmp(argv[2], "help"))
    return help(inv);

  FILE* f = 0;
  FILE* in = stdin;
  FILE* out = stdout;
  ucs4_t* buf = 0;

  for (int i = 2; i < argc; ++i) {
    if (!strcmp(argv[i], "-i")) {
      if (i + 1 == argc) {
        errno = EINVAL;
        goto err;
      }

      in = fopen(argv[++i], "r");

      if (!in) {
        errno = EIO;
        goto err;
      }
    } else if (!strcmp(argv[i], "-o")) {
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

      buf = malloc(sizeof(ucs4_t) * FMAX);

      if (!buf) {
        errno = ENOMEM;
        goto err;
      }

      key k;
      int bs;

      if (u8_get(f, N, buf) < N)
        goto err;

      if (key_fruc(k, buf, inv))
        goto err;

      if ((bs = u8_get(in, 0, buf)) == -1)
        goto err;

      if ((bs = encode(bs, buf, k)) == -1)
        goto err;

      if (u8_put(out, bs, buf) != bs)
        goto err;

      goto end;
    }
  }

  errno = EPROTO;

err:
  if (buf)
    free(buf);

  if (f)
    fclose(f);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  return -1;

end:
  if (buf)
    free(buf);

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
