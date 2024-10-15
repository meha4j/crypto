#ifndef FIT_H
#define FIT_H

#include <stdio.h>
#include <unitypes.h>

struct chromo {
  uint8_t* kr;
  double n;
};

int fit_init(FILE* f);
int fit(size_t gc, size_t ts, size_t ps, size_t ds, ucs4_t data[ds],
        struct chromo* bc);

#endif  // FIT_H
