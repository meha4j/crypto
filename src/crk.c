#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/random.h>
#include <unitypes.h>

#define N 33
#define ORG 1072

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
