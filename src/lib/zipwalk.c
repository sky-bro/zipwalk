#include <stdio.h>
#include <zlib.h>
#include "zipwalk.h"

// todo: run crc32 on inflated data
// inflate as much as possible, and stop as soon as any error is encountered
unsigned long my_inflate(FILE* fin, FILE* fout, unsigned char *src, unsigned long src_len, unsigned char *dst, unsigned long dst_len) {
    z_stream strm;
    strm.zalloc = 0;
    strm.zfree = 0;
    strm.opaque = 0;
    strm.avail_in = 0;
    if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
        fprintf(stderr, "[-] init fail!\n");
        return -1;
    }
    unsigned long len_in = 0, len_out = 0;
    int cnt = 0;
    while (1) {
        // printf("[*] loop cnt: %d\n", ++cnt);
        if (strm.avail_in == 0) {
            int sz = fread(src, sizeof(unsigned char), src_len, fin);
            if (sz == 0) break;
            strm.next_in = src;
            strm.avail_in = sz;
        }
        int sz = strm.avail_in;
        strm.avail_out = dst_len;
        strm.next_out = dst;
        int ret = inflate(&strm, Z_NO_FLUSH);
        // printf("[*] ret: %d\n", ret);
        len_in += sz - strm.avail_in;
        len_out += dst_len - strm.avail_out;
        printf("[*] len_in: %lu, len_out: %lu\n", len_in, len_out);
        fwrite(dst, sizeof(unsigned char), dst_len - strm.avail_out, fout);
        if (ret != Z_OK) {
            break;
        }
    }
    printf("[+] %lu bytes inflated to %lu bytes\n", len_in, len_out);
    (void)inflateEnd(&strm);
    return len_in;
}
