#include <cphr.h>
#include <ga.h>
#include <uc.h>

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>

#define SS 10000

static double seed[N];

int ga_seed(FILE* f) {
  if (!f) {
    errno = ENOENT;
    return -1;
  }

  for (int i = 0; i < N; ++i)
    if (fscanf(f, "%lf", &seed[i]) != 1) {
      fclose(f);
      errno = EIO;
      return -1;
    }

  return 0;
}

static inline void swap(double* fit, uint8_t** pop, size_t a, size_t b) {
  if (a == b)
    return;

  double ftmp = fit[a];
  uint8_t* ptmp = pop[a];

  fit[a] = fit[b];
  fit[b] = ftmp;

  pop[a] = pop[b];
  pop[b] = ptmp;
}

static int est(size_t ds, ucs4_t data[ds], double e[N]) {
  memset(e, 0, sizeof(double) * N);

  for (size_t i = 0; i < ds; ++i)
    if (A <= data[i] && data[i] < A + N)
      e[data[i] - A] += 1;
    else
      ds--;

  for (int i = 0; i < N; ++i)
    e[i] /= ds;

  return 0;
}

static int fit(double e[N], struct chr* chr) {
  chr->f = 0;

  for (int i = 0; i < N; ++i) {
    double diff = seed[i] - e[chr->k[i]];
    chr->f += diff * diff;
  }

  return 0;
}

static int rw_select(size_t ps, struct chr* pop, size_t ss, size_t sct[ss]) {
  double n = 0;

  for (size_t i = 0; i < ps; ++i) {
    pop[i].f = ss / pop[i].f;
    n += pop[i].f;
  }

  double k = ss / n;
  size_t ssp = 0;

  for (size_t i = 0; i < ps - 1; ++i) {
    size_t p = (size_t)(pop[i].f * k);

    for (int j = ssp; j < ssp + p; ++j)
      sct[j] = i;

    ssp += p;
  }

  for (int j = ssp; j < ss; ++j)
    sct[j] = ps - 1;

  return 0;
}

static int ord_cross(struct chr* a, struct chr* b, struct chr* c) {
  uint8_t rnd[4];

  if (getrandom(rnd, 4, 0) == -1) {
    errno = EIO;
    return -1;
  }

  int p = 0;

  for (int i = 0; i < 4; ++i) {
    p = rnd[i] % (N - p) + p;

    if (p == N)
      break;

    for (int j = p; j < 4 && j < N; ++j)
      c->k[j] = a->k[j];
  }

  return 0;
}

static int mut(struct chr* c) {
  uint8_t rnd[2];

  if (getrandom(rnd, 2, 0) == -1) {
    errno = EIO;
    return -1;
  }

  rnd[0] = rnd[0] % N;
  rnd[1] = rnd[1] % N;

  uint8_t tmp = c->k[rnd[0]];
  c->k[rnd[0]] = c->k[rnd[1]];
  c->k[rnd[1]] = tmp;

  return 0;
}

int ga_evo(size_t gc, size_t ps, int v, size_t ds, ucs4_t data[ds],
           struct chr* chr) {
  double e[N];
  ucs4_t b[N];

  est(ds, data, e);

  struct chr* pop = malloc(sizeof(struct chr) * ps);
  size_t* sct = malloc(sizeof(size_t) * SS);
  size_t* rnd = malloc(sizeof(size_t) * ps * 2);

  for (size_t i = 0; i < ps; ++i) {
    key_gen(pop[i].k);
    fit(e, &pop[i]);
  }

  chr->f = pop[0].f;

  for (int i = 0; i < N; ++i)
    chr->k[i] = pop[0].k[i];

  for (size_t i = 0; i < gc; ++i) {
    rw_select(ps, pop, SS, sct);

    if (getrandom(rnd, sizeof(size_t) * ps * 2, 0) == -1) {
      errno = EIO;
      return -1;
    }

    struct chr* npop = malloc(sizeof(struct chr) * ps);

    for (size_t j = 0; j < ps; ++j) {
      size_t f = sct[rnd[j * 2] % SS];
      size_t s = sct[rnd[j * 2 + 1] % SS];

      if (v) {
        key_touc(pop[f].k, b);
        u8_put(stdout, N, b);
        putchar('\n');
      }

      if (v) {
        key_touc(pop[s].k, b);
        u8_put(stdout, N, b);
        putchar('\n');
      }

      ord_cross(&pop[f], &pop[s], &npop[j]);

      if (v) {
        key_touc(npop[j].k, b);
        u8_put(stdout, N, b);
        putchar('\n');
      }

      putchar('\n');

      if (j == 10)
        return 0;

      mut(&npop[j]);
      fit(e, &npop[j]);

      if (npop[j].f < chr->f) {
        chr->f = npop[i].f;

        for (int k = 0; k < N; ++k)
          chr->k[k] = npop[j].k[k];
      }
    }

    free(pop);
    pop = npop;
  }

  free(pop);
  free(sct);
  free(rnd);

  return 0;
}

static int help() {
  printf("Usage: crk [OPTIONS]\n\n");
  printf("Available options:\n");
  printf("\t-i INFILE\tFile to decode, default: stdin.\n");
  printf("\t-o OUTFILE\tOutput file, default: stdout.\n");
  printf("\t-g GEN\t\tNumber of generations, default: 100.\n");
  printf("\t-p POP\t\tPopulation size, default: 100.\n");
  printf("\t-s SEEDFILE\tSeed, default: seed.data.\n");
  printf("\t-v\t\tVerbose.\n");

  return 0;
}

int crk_exe(int argc, char* argv[argc]) {
  FILE* seed = 0;
  FILE* in = stdin;
  FILE* out = stdout;

  ucs4_t* data = 0;

  size_t g = 100;
  size_t p = 100;

  int v = 0;

  for (int i = 2; i < argc; ++i) {
    if (!strcmp(argv[i], "help")) {
      help();
      goto end;
    } else if (!strcmp(argv[i], "-i")) {
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

      g = strtoul(argv[++i], NULL, 10);

      if (g == 0 || g == ULLONG_MAX) {
        errno = EINVAL;
        goto err;
      }
    } else if (!strcmp(argv[i], "-p")) {
      if (i + 1 == argc) {
        errno = EINVAL;
        goto err;
      }

      p = strtoul(argv[++i], NULL, 10);

      if (g == 0 || g == ULLONG_MAX) {
        errno = EINVAL;
        goto err;
      }
    } else if (!strcmp(argv[i], "-s")) {
      if (i + 1 == argc) {
        errno = EINVAL;
        goto err;
      }

      seed = fopen(argv[++i], "r");

      if (!seed) {
        errno = EIO;
        goto err;
      }
    } else if (!strcmp(argv[i], "-v")) {
      v = 1;
    } else {
      errno = EINVAL;
      goto err;
    }
  }

  if (!seed) {
    seed = fopen("seed.data", "r");

    if (!seed) {
      errno = EIO;
      goto err;
    }
  }

  ga_seed(seed);

  data = malloc(sizeof(ucs4_t) * FMAX);
  size_t ds;

  if ((ds = u8_get(in, 0, data)) == -1)
    goto err;

  struct chr c;

  if (ga_evo(g, p, v, ds, data, &c))
    goto err;

  if (key_touc(c.k, data))
    goto err;

  if (u8_put(out, N, data) != N)
    goto err;

  fprintf(out, " (%.2lf)\n", c.f);

end:
  if (data)
    free(data);

  if (seed)
    fclose(seed);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  return 0;

err:
  if (data)
    free(data);

  if (seed)
    fclose(seed);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  return -1;
}
