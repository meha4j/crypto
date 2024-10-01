// burro key KEYFILE

// burro enc KEYFILE
//  OPTIONS:
//    -in     Input file, default: stdin.
//    -out    Output file, default: stdout.

// burro dec [OPTIONS] KEYFILE
//  OPTIONS:
//    -in     Input file, default: stdin.
//    -out    Output file, default: stdout.

// burro crk [OPTIONS]
//  OPTIONS:
//    -v      Verbose.
//    -in     Input file, default: stdin.
//    -out    Output file, default: stdout.

#include <errno.h>
#include <string.h>

#include "crk.h"
#include "enc.h"
#include "key.h"

int help() {
  printf("Usage: burro COMMAND\n\n");
  printf("COMMAND\tCommand to execute.\n\n");
  printf("Available commands:\n");
  printf("\tkey\tKey management.\n");
  printf("\tenc\tEncode text using private key.\n");
  printf("\tdec\tDecode text using private key.\n");
  printf("\tcrk\tDecode text without private key.\n");

  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2 || !strcmp(argv[1], "help"))
    return help();

  int r;

  if (!strcmp(argv[1], "key"))
    r = key_exe(argc, argv);
  else if (!strcmp(argv[1], "enc"))
    r = enc_exe(argc, argv);
  else if (!strcmp(argv[1], "dec"))
    r = dec_exe(argc, argv);
  else if (!strcmp(argv[1], "crk"))
    r = crk_exe(argc, argv);
  else {
    printf("Unknown command: %s.\n", argv[1]);
    return help();
  }

  if (r) {
    printf("fatal: %s.\n", strerror(errno));
    return -1;
  }

  return 0;
}
