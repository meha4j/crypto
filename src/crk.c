#include <errno.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <unitypes.h>

#include "key.h"

#define N 33
#define ORG 1072
#define POP 100

double trg[N];

int crk_init() {
  FILE* f = fopen("target.data", "r");

  if (!f) {
    errno = ENOENT;
    return -1;
  }

  for (int i = 0; i < N; ++i)
    if (fscanf(f, "%lf", &trg[i]) != 1) {
      fclose(f);
      errno = EIO;
      return -1;
    }

  fclose(f);
  return 0;
}

int crk_est(size_t s, ucs4_t raw[s], double frq[N]) {
  memset(frq, 0, N * sizeof(double));

  for (int i = 0; i < s; ++i)
    frq[raw[i] - ORG] += 1;

  for (int i = 0; i < N; ++i)
    frq[i] /= (double)s;

  return 0;
}

int crk_fit(uint8_t key[N], double frq[N], double* rate) {
  *rate = 0;

  for (int i = 0; i < N; ++i) {
    double diff = trg[i] - frq[key[i]];
    *rate += diff * diff;
  }

  return 0;
}

int crk_mut(uint8_t key[N]) {
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

int crk_cross(uint8_t a[N], uint8_t b[N], uint8_t c[N]) {
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

int crk_slct(size_t ps, double fit[ps], uint8_t* pop[ps], size_t n) {
  if (n > ps) {
    errno = EINVAL;
    return -1;
  }

  for (size_t i = 0; i < n; ++i) {
    double min = DBL_MAX;
    size_t imin = SIZE_MAX;

    for (size_t j = i; j < n; ++j)
      if (fit[j] < min) {
        imin = j;
        min = fit[j];
      }

    swap(fit, pop, i, imin);
  }

  return 0;
}

uint8_t** pop_new(size_t ps) {
  uint8_t** pop = malloc(sizeof(uint8_t*) * ps);

  for (size_t i = 0; i < ps; ++i)
    pop[i] = malloc(N);

  return pop;
}

void pop_est(size_t ps, uint8_t* pop[ps], double rate[ps], double frq[N]) {
  for (size_t i = 0; i < ps; ++i)
    crk_fit(pop[i], frq, &rate[i]);
}

void pop_free(size_t ps, uint8_t* pop[ps]) {
  for (size_t i = 0; i < ps; ++i)
    free(pop[i]);

  free(pop);
}

int crk_run(size_t r, size_t s, ucs4_t raw[s]) {
  crk_init();

  double* frq = malloc(sizeof(double) * s);
  double rate[POP];

  crk_est(s, raw, frq);

  uint8_t** pop = pop_new(POP);

  for (size_t i = 0; i < POP; ++i)
    key_gen(pop[i], N);

  for (int i = 0; i < r; ++i) {
    pop_est(POP, pop, rate, frq);
    crk_slct(POP, rate, pop, 14);

    uint8_t** npop = pop_new(POP);

    size_t s = 0;

    for (int j = 0; j < 13 && s < POP; ++j)
      for (int k = j + 1; k < 14 && s < POP; ++k)
        crk_cross(pop[j], pop[k], npop[s++]);

    pop_free(POP, pop);
    pop = npop;
  }

  free(frq);

  return 0;
}
