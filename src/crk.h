#ifndef CRK_H
#define CRK_H

#include <stdio.h>
#include <unitypes.h>

int crk_seed(FILE* f);
int crk(size_t sgen, size_t spop, size_t ds, ucs4_t data[ds]);

int crk_exe(int argc, char* argv[argc]);

#endif  // CRK_H
