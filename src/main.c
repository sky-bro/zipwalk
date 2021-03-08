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
                            {"extension", required_argument, 0, 'e'},
                            {0, 0, 0, 0}};

const char* program_name;

void printUsage() {
  printf("Usage: %s [-vsh] [-t target_path] -f <file_path>\n", program_name);
}

FILE* fin = NULL;
int save_file = 0; // will save files or just extract headers
int custom_filetype = 0;
int filetype = ZIP_FILE; // default to zip file type

int parse_args(int argc, char *const argv[]) {
  program_name = argv[0];
  // opterr = 0;
  int c;
  char *file_path = NULL, *target_path = NULL;
  char target_path_buf[256];
  while ((c = getopt_long(argc, argv, "vhst:f:", longopts, NULL)) != -1) {
    switch (c) {
      case 'v':
        printf("zipwalk - version %d.%d\n", VERSION_MAJOR, VERSION_MINOR);
        exit(0);
        break;
      case 't':
        // printf("target_path: %s\n", optarg);
        // ignore this for now, always extract xxx/filename.zip to xx/filename/ folder
        // xxx/filename to xxx/ folder
        target_path = optarg;
        break;
      case 'f':
        printf("zip file: %s\n", optarg);
        file_path = optarg;
        break;
      case 's':
        save_file = 1;
        break;
      case 'e':
        custom_filetype = 1;
        if (optarg[0] == 't') filetype = TAR_FILE;
        else if (optarg[0] == 'g') filetype = GZIP_FILE;
        else filetype = ZIP_FILE;
        break;
      case 'h':
      default:
        return 1;
    }
    
  }
  if (!file_path) {
    return 1;
  }

  struct stat statbuf;
  if (stat(file_path, &statbuf)) {
    fprintf(stderr, "%s does not exist\n", file_path);
    return 1;
  }
  if (!S_ISREG(statbuf.st_mode)) { // why we can fopen a directory ??
    fprintf(stderr, "%s is not a regular file.\n", file_path);
    return 1;
  }
  if (custom_filetype) filetype = get_filetype(file_path);
  fin = fopen(file_path, "rb");
  if (!fin) {
    perror("open file_path error");
    return 1;
  }

  // mkdir, chdir to target_path
  if (target_path) {
    if (my_mkdir(target_path, 0777)) {
      fclose(fin);
      fin = NULL;
      return 1;
    }
    chdir(target_path);
  } else {
    char *p = strrchr(file_path, '/');
    if (p) {
      *p = 0;
      chdir(file_path);
      *p = '/';
    }
  }
  char buf[MAX_PATH_LEN];
  printf("[*] now in directory: %s\n", getcwd(buf, MAX_PATH_LEN));
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
  switch (filetype)
  {
  case GZIP_FILE:
    if (parse_GZIP_header(fin, save_file)) { // success
      parse_GZIP_footer(fin);
    }
    break;
  case TAR_FILE:
    fprintf(stderr, "TODO: parse TAR_FILE\n");
    break;
  case ZIP_FILE:
  default:
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
    break;
  }
  fclose(fin);
  return 0;
}
