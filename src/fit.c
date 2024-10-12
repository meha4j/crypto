#include <fit.h>
#include <rot.h>
#include <uc.h>

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>

static double seed[KS][KS];

int init(FILE* f) {
  if (!f) {
    errno = EINVAL;
    return -1;
  }

  for (int i = 0; i < KS; ++i)
    for (int j = 0; j < KS; ++j)
      if (fscanf(f, "%lf", &seed[i][j]) != 1) {
        fclose(f);
        return -1;
      }

  return 0;
}

static int nrm(double tg[KS][KS], struct chromo* chr) {
  chr->n = 0;

  for (int i = 0; i < KS; ++i)
    for (int j = 0; j < KS; ++j) {
      double d = seed[i][j] - tg[chr->kr[i]][chr->kr[j]];
      chr->n += d * d;
    }

  return 0;
}

static int tour(size_t ts, size_t ps, struct chromo* pop, struct chromo** os) {
  if (ts > ps)
    return -1;

  size_t* rnd = malloc(sizeof(size_t) * ts);

  if (getrandom(rnd, sizeof(size_t) * ts, 0) == -1) {
    free(rnd);
    return -1;
  }

  struct chromo* bc;

  for (size_t i = 0; i < ts; ++i)
    rnd[i] = rnd[i] % ps;

  bc = &(pop[rnd[0]]);

  for (size_t i = 1; i < ts; ++i)
    if (pop[rnd[i]].n < bc->n)
      bc = &(pop[rnd[i]]);

  *os = bc;

  free(rnd);
  return 0;
}

static int ox1(struct chromo* a, struct chromo* b, struct chromo* os) {
  uint8_t rnd[2];

  if (getrandom(rnd, 2, 0) == -1)
    return -1;

  rnd[0] = rnd[0] % KS;
  rnd[1] = rnd[1] % KS;

  if (rnd[0] > rnd[1]) {
    uint8_t t = rnd[0];
    rnd[0] = rnd[1];
    rnd[1] = t;
  }

  uint64_t f = 0UL;

  for (int i = rnd[0]; i < rnd[1]; ++i) {
    os->kr[i] = a->kr[i];
    f |= (1UL << os->kr[i]);
  }

  int op = rnd[1];
  int bp = rnd[1];

  do {
    if (!(f & (1UL << b->kr[bp]))) {
      os->kr[op] = b->kr[bp];

      if (op == KS - 1)
        op = 0;
      else
        op++;
    }

    if (bp == KS - 1)
      bp = 0;
    else
      bp++;
  } while (op != rnd[0]);

  return 0;
}

static int mut(struct chromo* chr) {
  uint8_t rnd[2];

  if (getrandom(rnd, 2, 0) == -1)
    return -1;

  rnd[0] = rnd[0] % KS;
  rnd[1] = rnd[1] % KS;

  uint8_t t = chr->kr[rnd[0]];

  chr->kr[rnd[0]] = chr->kr[rnd[1]];
  chr->kr[rnd[1]] = t;

  return 0;
}

static int est(double tg[KS][KS], size_t ds, ucs4_t* data) {
  if (ds < 2)
    return -1;

  for (int i = 0; i < KS; ++i)
    memset(tg[i], 0, sizeof(double) * KS);

  double s = 0;
  ucs4_t p = data[0];

  for (size_t i = 1; i < ds; ++i) {
    if ((data[i] < A) || ((A + KS) <= data[i]))
      continue;

    if ((p < A) || ((A + KS) <= p)) {
      p = data[i];
      continue;
    }

    tg[p - A][data[i] - A] += 1;
    p = data[i];
    s += 1;
  }

  for (int i = 0; i < KS; ++i)
    for (int j = 0; j < KS; ++j)
      tg[i][j] /= s;

  return 0;
}

static struct chromo* pgen(size_t ps) {
  struct chromo* pop = malloc(sizeof(struct chromo) * ps);

  if (!pop)
    return 0;

  for (size_t i = 0; i < ps; ++i) {
    pop[i].kr = malloc(KS);

    if (!pop[i].kr) {
      errno = ENOMEM;

      for (size_t j = 0; j < i; ++j)
        free(pop[j].kr);

      free(pop);
      return 0;
    }
  }

  return pop;
}

void pfree(size_t ps, struct chromo* pop) {
  for (size_t i = 0; i < ps; ++i)
    free(pop[i].kr);

  free(pop);
}

int fit(size_t gc, size_t ts, size_t ps, size_t ds, ucs4_t* data,
        struct chromo* bc) {
  double tg[KS][KS];

  if (est(tg, ds, data))
    return -1;

  struct chromo* cpop = pgen(ps);

  if (!cpop)
    return -1;

  for (size_t i = 0; i < ps; ++i) {
    if (kgen(cpop[i].kr)) {
      pfree(ps, cpop);
      return -1;
    }

    if (nrm(tg, &(cpop[i]))) {
      pfree(ps, cpop);
      return -1;
    }
  }

  memcpy(bc->kr, cpop[0].kr, KS);
  bc->n = cpop[0].n;

  for (size_t g = 0; g < gc; ++g) {
    struct chromo* ppop = cpop;
    struct chromo* gbc = 0;
    struct chromo* a;
    struct chromo* b;

    cpop = pgen(ps);

    if (!cpop) {
      pfree(ps, ppop);
      return -1;
    }

    for (size_t i = 0; i < ps; ++i) {
      if (tour(ts, ps, ppop, &a)) {
        pfree(ps, ppop);
        pfree(ps, cpop);
        return -1;
      }

      if (tour(ts, ps, ppop, &b)) {
        pfree(ps, ppop);
        pfree(ps, cpop);
        return -1;
      }

      if (ox1(a, b, &(cpop[i]))) {
        pfree(ps, ppop);
        pfree(ps, cpop);
        return -1;
      }

      mut(&(cpop[i]));
      nrm(tg, &(cpop[i]));

      if (gbc == 0 || gbc->n > cpop[i].n)
        gbc = &cpop[i];
    }

    if (gbc->n < bc->n) {
      memcpy(bc->kr, gbc->kr, KS);
      bc->n = gbc->n;
    }

    pfree(ps, ppop);
  }

  return 0;
}
