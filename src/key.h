#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include <stdio.h>

int key_gen(uint8_t* key, uint8_t n);
int key_put(FILE* f, uint8_t* key, uint8_t n);
int key_get(FILE* f, uint8_t* key, uint8_t n, int inv);

#endif  // KEY_H
