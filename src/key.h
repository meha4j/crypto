#ifndef KEY_H
#define KEY_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define ALPH_SIZE 33
#define ALPH_BEG 1072
#define KEY_LEN 66

int key_get(FILE* f, uint8_t key[ALPH_SIZE], bool inv);
int key_exe(int argc, char* argv[argc]);

#endif  // KEY_H
