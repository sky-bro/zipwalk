#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "zipwalk.h"

struct option longopts[] = {{"version", no_argument, 0, 'v'},
                            {"help", no_argument, 0, 'h'},
                            {"target_path", required_argument, 0, 't'},
                            {"file", required_argument, 0, 'f'},
                            {0, 0, 0, 0}};

const char* program_name;

void printUsage() {
  printf("Usage: %s [-vh] [-t target_path] -f <zip_path>\n", program_name);
}

const char *usage = "%s -vhtf\n";

int main(int argc, char *const argv[]) {
  program_name = argv[0];
  // opterr = 0;
  int c;
  char *zip_path = NULL, *target_path = NULL;
  char target_path_buf[256];
  while ((c = getopt_long(argc, argv, "vht:f:", longopts, NULL)) != -1) {
    switch (c) {
      case 'v':
        printf("zipwalk - version %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
        return 0;
        break;
      case 't':
        printf("target_path: %s\n", optarg);
        // ignore this for now, always extract xxx/filename.zip to xx/filename/ folder
        // xxx/filename to xxx/ folder
        target_path = optarg;
        break;
      case 'f':
        printf("zip file: %s\n", optarg);
        zip_path = optarg;
        break;
      case 'h':
      default:
        printUsage();
        return 0;
    }
    
  }
  if (!zip_path) {
    printUsage();
    return 0;
  }

  struct stat statbuf;
  if (stat(zip_path, &statbuf)) {
    fprintf(stderr, "%s does not exist\n", zip_path);
    return 1;
  }
  if (!S_ISREG(statbuf.st_mode)) { // why we can fopen a directory ??
    fprintf(stderr, "%s is not a regular file.\n", zip_path);
    return 1;
  }
  FILE* fp = fopen(zip_path, "rb");
  if (!fp) {
    perror("open zip_path error");
    return 1;
  }

  // mkdir, chdir
  char *p = strrchr(zip_path, '/');
  if (p) {
    p[1] = 0;
    chdir(zip_path);
  }

  fseek(fp, 0, SEEK_END);
  printf("file size: %ld bytes\n", ftell(fp));
  rewind(fp);

  // system("pwd");
  return 0;
}
