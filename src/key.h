#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include <stdio.h>

#define N 33
#define A 1072

int key_gen(uint8_t* key);
int key_put(FILE* f, uint8_t* key);
int key_get(FILE* f, uint8_t* key, int inv);

int key_exe(int argc, char* argv[argc]);

#endif  // KEY_H
