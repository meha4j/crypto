#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unitypes.h>

#include "enc.h"
#include "key.h"
#include "u8.h"

#define N 33

int main(int argc, char* argv[]) {
  setlocale(LC_CTYPE, "en_US.UTF-8");

  FILE* f = fopen("ru.key", "r");
  uint8_t key[N];

  if (key_get(f, key, N, 1)) {
    printf("[key_get] fatal: %s.", strerror(errno));
    fclose(f);
    return -1;
  }

  fclose(f);

  ucs4_t* raw;
  size_t s;

  if (u8_get(stdin, &raw, &s)) {
    printf("[u8_get] fatal: %s.", strerror(errno));
    return -1;
  }

  ucs4_t* eucs;
  size_t es;

  if (enc(raw, s, key, N, &eucs, &es)) {
    printf("[enc] fatal: %s.", strerror(errno));
    free(raw);
    return -1;
  }

  if (u8_put(stdout, eucs, es)) {
    printf("[u8_put] fatal: %s.", strerror(errno));
    free(raw);
    free(eucs);
    return -1;
  }

  free(raw);
  free(eucs);

  return 0;
}
