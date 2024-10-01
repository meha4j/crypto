#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <unistr.h>
#include <unitypes.h>

#include "crk.h"
#include "key.h"
#include "log.h"
#include "u8.h"

static double seed[N];

static int crk_seed(FILE* f) {
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

static int crk_est(size_t ds, ucs4_t data[ds], double est[N]) {
  for (int i = 0; i < N; ++i)
    est[i] = 0.0;

  for (size_t i = 0; i < ds; ++i)
    if (data[i] >= A && data[i] < A + N)
      est[data[i] - A] += 1;

  log_debug("Estimation: ");

  char buf[5];
  int s;

  for (int i = 0; i < N; ++i) {
    est[i] /= (double)ds;
    s = u8_uctomb((uint8_t*)buf, est[i], 4);
    buf[s] = '\n';
    printf("%s\t%lf\n", buf, est[i] * 100);
  }

  return 0;
}

static int crk_fit(uint8_t key[N], double est[N], double* rate) {
  *rate = 0;

  for (int i = 0; i < N; ++i) {
    double diff = seed[i] - est[key[i]];
    *rate += diff * diff;
  }

  return 0;
}

static int crk_mut(uint8_t key[N]) {
  uint8_t rnd[2];

  if (getrandom(rnd, 2, 0) == -1) {
    errno = EIO;
    return -1;
  }

  rnd[0] = rnd[0] % N;
  rnd[1] = rnd[1] % N;

  uint8_t tmp = key[rnd[0]];
  key[rnd[0]] = key[rnd[1]];
  key[rnd[1]] = tmp;

  return 0;
}

static int crk_cross(uint8_t a[N], uint8_t b[N], uint8_t c[N]) {
  uint64_t f = 0;
  uint8_t rnd[N];

  if (getrandom(rnd, N, 0) == -1) {
    errno = EIO;
    return -1;
  }

  for (int i = 0; i < N; ++i) {
    if (f & (1 << a[i]))
      c[i] = b[i];
    else if (f & (1 << b[i]))
      c[i] = a[i];
    else
      c[i] = rnd[i] % 2 == 0 ? a[i] : b[i];

    f = f | (1 << c[i]);
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

static int crk_slct(size_t ss, size_t ps, double fit[ps], uint8_t* pop[ps]) {
  if (ss > ps) {
    errno = EINVAL;
    return -1;
  }

  for (size_t i = 0; i < ss; ++i) {
    double min = DBL_MAX;
    size_t imin = SIZE_MAX;

    for (size_t j = i; j < ss; ++j)
      if (fit[j] < min) {
        imin = j;
        min = fit[j];
      }

    swap(fit, pop, i, imin);
  }

  return 0;
}

static uint8_t** pop_new(size_t ps) {
  uint8_t** pop = malloc(sizeof(uint8_t*) * ps);

  for (size_t i = 0; i < ps; ++i)
    pop[i] = malloc(N);

  return pop;
}

static void pop_fit(double est[N], size_t ps, uint8_t* pop[ps],
                    double fit[ps]) {
  for (size_t i = 0; i < ps; ++i)
    crk_fit(pop[i], est, &fit[i]);
}

static void pop_free(size_t ps, uint8_t* pop[ps]) {
  for (size_t i = 0; i < ps; ++i)
    free(pop[i]);

  free(pop);
}

static int crk(size_t g, size_t p, int v, size_t ds, ucs4_t data[ds]) {
  double est[N];

  double* fit = malloc(sizeof(double) * p);
  uint8_t** pop;

  log_debug("Estimating data [%d]...", ds);
  crk_est(ds, data, est);

  pop = pop_new(p);

  log_debug("Generating initial population [%d]...", p);
  for (size_t i = 0; i < p; ++i)
    key_gen(pop[i]);

  log_debug("Starting...");
  for (size_t i = 0; i < g; ++i) {
    log_debug("Fitting...");
    pop_fit(est, p, pop, fit);

    log_debug("Selecting [%d]...", p / 2);
    crk_slct(p / 2, p, fit, pop);

    log_debug("Best fit: %lf.", fit[0]);

    if (v) {
      key_put(stdout, pop[0]);
      putchar('\n');
    }

    uint8_t** npop = pop_new(p);

    log_debug("Crossing & mutating chromosomes...");
    for (size_t j = 0, s = 0; j < p - 1 && s < p; ++j)
      for (size_t k = j + 1; k < p && s < p; ++k) {
        crk_cross(pop[j], pop[k], npop[s]);
        crk_mut(npop[s]);

        s += 1;
      }

    pop_free(p, pop);
    pop = npop;
  }

  log_debug("Selecting [%d]...", 1);
  crk_slct(1, p, fit, pop);
  printf("Fittest one: ");
  key_put(stdout, pop[0]);

  pop_free(p, pop);
  free(fit);

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

  crk_seed(seed);

  ucs4_t* data;
  size_t ds;

  if (u8_get(in, &data, &ds)) {
    goto err;
  }

  if (crk(g, p, v, ds, data)) {
    goto err;
  }

end:
  if (seed)
    fclose(seed);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  return 0;

err:
  if (seed)
    fclose(seed);

  if (in && in != stdin)
    fclose(in);

  if (out && out != stdout)
    fclose(out);

  return -1;
}
