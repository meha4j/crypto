#include <errno.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <unitypes.h>

#include "crk.h"
#include "key.h"

double seed[N];

int crk_seed(FILE* f) {
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
  memset(est, 0, N * sizeof(double));

  for (int i = 0; i < ds; ++i)
    est[data[i] - A] += 1;

  for (int i = 0; i < N; ++i)
    est[i] /= (double)ds;

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
  uint8_t f = 0;
  uint8_t rnd[N];

  if (getrandom(rnd, N, 0) == -1) {
    errno = EIO;
    return -1;
  }

  for (int i = 0; i < N; ++i) {
    if (f & (1 << (N - 1 - a[i])))
      c[i] = b[i];
    else if (f & (1 << (N - 1 - b[i])))
      c[i] = a[i];
    else
      c[i] = rnd[i] % 2 == 0 ? a[i] : b[i];

    f |= (1 << (N - 1 - c[i]));
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

int crk(size_t sgen, size_t spop, size_t ds, ucs4_t data[ds]) {
  double est[N];

  double* fit = malloc(sizeof(double) * spop);
  uint8_t** pop;

  crk_est(ds, data, est);

  pop = pop_new(spop);

  for (size_t i = 0; i < spop; ++i)
    key_gen(pop[i]);

  for (int i = 0; i < sgen; ++i) {
    pop_fit(est, spop, pop, fit);
    crk_slct(spop / 2, spop, fit, pop);

    uint8_t** npop = pop_new(spop);

    for (int j = 0, s = 0; j < spop - 1 && s < spop; ++j)
      for (int k = j + 1; k < spop && s < spop; ++k)
        crk_cross(pop[j], pop[k], npop[s]);

    pop_free(spop, pop);
    pop = npop;
  }

  pop_free(spop, pop);
  free(fit);

  return 0;
}

int crk_exe(int argc, char** argv) {
  return 0;
}
