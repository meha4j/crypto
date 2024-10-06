#ifndef GA_H
#define GA_H

#include <cphr.h>
#include <stdio.h>
#include <unitypes.h>

struct chr {
  key k;
  double f;
};

int ga_seed(FILE* f);
int ga_evo(size_t gc, size_t ps, int v, size_t ds, ucs4_t data[ds], struct chr* chr);

#endif  // GA_H
