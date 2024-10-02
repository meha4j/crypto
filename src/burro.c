#include <errno.h>
#include <stdio.h>
#include <string.h>

int key_exe(int argc, char** argv);
int enc_exe(int argc, char** argv);
int dec_exe(int argc, char** argv);
int crk_exe(int argc, char** argv);

int help() {
  printf("Usage: burro COMMAND\n\n");
  printf("COMMAND\tCommand to execute.\n\n");
  printf("Available commands:\n");
  printf("\tkey\tKey generation.\n");
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
    errno = ENOTSUP;
    r = -1;
  }

  if (r) {
    printf("error: %s. (try: burro [COMMAND] help)\n", strerror(errno));
    return -1;
  }

  return 0;
}
