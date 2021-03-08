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
                            {"save-file", no_argument, 0, 's'},
                            {0, 0, 0, 0}};

const char* program_name;

void printUsage() {
  printf("Usage: %s [-vsh] [-t target_path] -f <zip_path>\n", program_name);
}

FILE* fin = NULL;
int save_file = 0; // will save files or just extract headers

int parse_args(int argc, char *const argv[]) {
  program_name = argv[0];
  // opterr = 0;
  int c;
  char *zip_path = NULL, *target_path = NULL;
  char target_path_buf[256];
  while ((c = getopt_long(argc, argv, "vhst:f:", longopts, NULL)) != -1) {
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
      case 's':
        save_file = 1;
        break;
      case 'h':
      default:
        return 1;
    }
    
  }
  if (!zip_path) {
    return 1;
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
  fin = fopen(zip_path, "rb");
  if (!fin) {
    perror("open zip_path error");
    return 1;
  }

  // mkdir, chdir
  char *p = strrchr(zip_path, '/');
  if (p) {
    p[1] = 0;
    chdir(zip_path);
  }
  char buf[MAX_PATH_LEN];
  printf("[*] target directory: %s\n", getcwd(buf, MAX_PATH_LEN));
  return 0;
}

int main(int argc, char *const argv[]) {
  if (parse_args(argc, argv)) {
    printUsage();
    return 0;
  }

  fseek(fin, 0, SEEK_END);
  printf("[*] file size: %ld bytes\n", ftell(fin));
  rewind(fin);

  int ret;
  while (ret = next_header(fin)) {
    switch (ret) {
    case CDFH: // Central directory file header
      parse_CDFH(fin);
      break;
    case LFH: // Local file header
      parse_LFH(fin, save_file);
      break;
    case EOCD: // End of central directory
      parse_EOCD(fin);
      break;
    case ODD: // Optional data descriptor
      parse_ODD(fin);
      break;
    default:
      fprintf(stderr, "[-] unknown zip header type: %d\n", ret);
      break;
    }
  }
  return 0;
}
