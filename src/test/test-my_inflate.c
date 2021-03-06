#include <zlib.h>
#include <stdio.h>
#include <stdlib.h>
#include "zipwalk.h"

int main(int argc, const char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s <123.deflate> <123.txt>\n", argv[0]);
    return -1;
  }
  FILE* fin = fopen(argv[1], "rb");
  if (!fin) {
    perror("[-] fail to fopen 123.deflate");
    return -1;
  }
  FILE* fout = fopen(argv[2], "wb");
  if (!fout) {
    perror("[-] fail to fopen 123.txt");
    return -1;
  }
  fseek(fin, 0, SEEK_END);
  unsigned long src_len = ftell(fin), dst_len = 1;
  printf("[+] File size: %lu\n", src_len);
  rewind(fin);
  unsigned char *src = malloc(src_len);
  unsigned char *dst = malloc(dst_len);
  my_inflate(fin, fout, src, 10, dst, dst_len);
  fclose(fin);
  fclose(fout);
  free(src);
  free(dst);
  return 0;
}
