#ifndef GA_H
#define GA_H

#include <stdio.h>
#include <unitypes.h>

int ga_seed(FILE* f);

int ga_exe(size_t gc, size_t ps, size_t ds, ucs4_t data[ds]);

#endif  // GA_H
