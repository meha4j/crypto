#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fit.h>
#include <rot.h>
#include <uc.h>

static int key_help() {
  printf("Usage: key KEYFILE\n\n");
  printf("KEYFILE\t\tOutput file.\n");

  return 0;
}

static int key_exe(int argc, char* argv[argc]) {
  if (argc < 3 || !strcmp(argv[2], "help"))
    return key_help();

  FILE* f = fopen(argv[2], "w+");

  if (!f) {
    errno = EIO;
    return -1;
  }

  ucs4_t data[KS];
  uint8_t k[KS];

  if (rot_kgen(k)) {
    fclose(f);
    return -1;
  }

  if (rot_k2uc(k, data)) {
    fclose(f);
    return -1;
  }

  if (u8_put(f, KS, data) != KS) {
    fclose(f);
    return -1;
  }

  fclose(f);
  return 0;
}

static int rot_help() {
  printf("Usage: rot [OPTIONS] KEYFILE\n\n");
  printf("KEYFILE\t\tFile with private key.\n\n");
  printf("Available options:\n");
  printf("\t-r\t\tInverse key.\n");
  printf("\t-i INFILE\tFile to rotate, default: stdin.\n");
  printf("\t-o OUTFILE\tOutput file, default: stdout.\n");

  return 0;
}

static int rot_exe(int argc, char* argv[argc]) {
  if (argc < 3 || !strcmp(argv[2], "help"))
    return rot_help();

  FILE* kf = 0;
  FILE* in = stdin;
  FILE* out = stdout;

  ucs4_t* buf = 0;
  int inv = 0;

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
    } else if (!strcmp(argv[i], "-r")) {
      inv = 1;
    } else {
      kf = fopen(argv[i], "r");

      if (!kf) {
        errno = EIO;
        goto err;
      }

      buf = malloc(sizeof(ucs4_t) * FMAX / 4);

      if (!buf) {
        errno = ENOMEM;
        goto err;
      }

      uint8_t k[KS];
      int bs;

      if (u8_get(kf, KS, buf) < KS)
        goto err;

      if (rot_uc2k(k, buf))
        goto err;

      if (inv && rot_kinv(k))
        goto err;

      if ((bs = u8_get(in, 0, buf)) == -1)
        goto err;

      if ((bs = rot(bs, buf, k)) == -1)
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

  if (kf)
    fclose(kf);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  return -1;

end:
  if (buf)
    free(buf);

  if (kf)
    fclose(kf);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  return 0;
}

static int fit_help() {
  printf("Usage: fit [OPTIONS]\n\n");
  printf("Available options:\n");
  printf("\t-g UINT\t\tNumber of generations, default: 100.\n");
  printf("\t-p UINT\t\tPopulation size, default: 100.\n");
  printf("\t-t UINT\t\tTournament size, default: 10.\n");
  printf("\t-i INFILE\tFile to fit, default: stdin.\n");
  printf("\t-o OUTFILE\tOutput file, default: stdout.\n");

  return 0;
}

static int fit_exe(int argc, char* argv[argc]) {
  if (argc < 3 || !strcmp(argv[2], "help"))
    return fit_help();

  FILE* in = stdin;
  FILE* out = stdout;
  FILE* sf = 0;

  size_t gc = 100;
  size_t ps = 100;
  size_t ts = 10;

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
    } else if (!strcmp(argv[i], "-g")) {
      if (i + 1 == argc) {
        errno = EINVAL;
        goto err;
      }

      gc = strtoul(argv[++i], NULL, 10);

      if (errno)
        goto err;
    } else if (!strcmp(argv[i], "-p")) {
      if (i + 1 == argc) {
        errno = EINVAL;
        goto err;
      }

      ps = strtoul(argv[++i], NULL, 10);

      if (errno)
        goto err;
    } else if (!strcmp(argv[i], "-t")) {
      if (i + 1 == argc) {
        errno = EINVAL;
        goto err;
      }

      ts = strtoul(argv[++i], NULL, 10);

      if (errno)
        goto err;
    }
  }

  sf = fopen("seed.data", "r");

  if (!sf)
    goto err;

  if (fit_init(sf))
    goto err;

  buf = malloc(sizeof(ucs4_t) * FMAX / 4);

  if (!buf) {
    errno = ENOMEM;
    goto err;
  }

  long bs;

  if ((bs = u8_get(in, 0, buf)) == -1L)
    goto err;

  struct chromo r;
  r.kr = malloc(KS);

  if (!r.kr) {
    errno = ENOMEM;
    goto err;
  }

  if (fit(gc, ts, ps, bs, buf, &r)) {
    free(r.kr);
    goto err;
  }

  if (rot_kinv(r.kr)) {
    free(r.kr);
    goto err;
  }

  if (rot_k2uc(r.kr, buf)) {
    free(r.kr);
    goto err;
  }

  if (u8_put(out, KS, buf) != KS) {
    free(r.kr);
    goto err;
  }

  if (out == stdout)
    putchar('\n');

  free(r.kr);

  if (buf)
    free(buf);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  if (sf)
    fclose(sf);

  return 0;

err:
  if (buf)
    free(buf);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  if (sf)
    fclose(sf);

  return -1;
}

int help() {
  printf("Usage: burro COMMAND\n\n");
  printf("COMMAND\tCommand to execute.\n\n");
  printf("Available commands:\n");
  printf("\tkey\tKey generation.\n");
  printf("\trot\tRotate text using private key.\n");
  printf("\tfit\tRotate text without private key.\n");

  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2 || !strcmp(argv[1], "help"))
    return help();

  int r;

  if (!strcmp(argv[1], "key"))
    r = key_exe(argc, argv);
  else if (!strcmp(argv[1], "rot"))
    r = rot_exe(argc, argv);
  else if (!strcmp(argv[1], "fit"))
    r = fit_exe(argc, argv);
  else {
    errno = ENOTSUP;
    r = -1;
  }

  if (r) {
    printf("error: %s. (try: burro [COMMAND] help)\n", strerror(errno));
    return -1;
  }

  return 0;
}
