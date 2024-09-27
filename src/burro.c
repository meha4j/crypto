#include <locale.h>
#include <string.h>

#include "enc.h"
#include "key.h"

int help() {
  printf("Usage: burro COMMAND\n\n");
  printf("COMMAND\t\tCommand to execute.\n\n");
  printf("Available commands:\n");
  printf("  key\t\tKey management.\n");
  printf("  enc\t\tEncode with key.\n");
  printf("  dec\t\tDecode with key.\n");
  printf("  crk\t\tDecode without key.\n");
  return 0;
}

int main(int argc, char* argv[]) {
  setlocale(LC_CTYPE, "en_US.UTF-8");

  if (argc < 2) {
    return help();
  }

  if (strcmp(argv[1], "key") == 0)
    return key_exe(argc, argv);
  else if (strcmp(argv[1], "enc") == 0 || strcmp(argv[1], "dec") == 0)
    return enc_exe(argc, argv);
  else
    return help();
}
