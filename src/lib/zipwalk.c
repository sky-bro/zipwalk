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

#define BUF_SIZE 1024
unsigned char buf[BUF_SIZE];
int next_header(FILE* fin) {
    int sz = 0;
    int i = 0;
    while (1) {
        if (sz - i < 4) {
            fseek(fin, i-sz, SEEK_CUR); // seek back
            sz = fread(buf, sizeof(unsigned char), BUF_SIZE, fin);
            if (sz < 4) break;
            i = 0;
        }
        if (buf[i] == 'P' && buf[i+1] == 'K') {
            if (buf[i+2] == 0x01 && buf[i+3] == 0x02) { // Central directory file header
                fseek(fin, i-sz, SEEK_CUR);
                return CDFH;
            }
            if (buf[i+2] == 0x03 && buf[i+3] == 0x04) { // Local file header
                fseek(fin, i-sz, SEEK_CUR);
                return LFH;
            }
            if (buf[i+2] == 0x05 && buf[i+3] == 0x06) { // End of central directory
                fseek(fin, i-sz, SEEK_CUR);
                return EOCD;
            }
            if (buf[i+2] == 0x07 && buf[i+3] == 0x08) { // Optional data descriptor
                fseek(fin, i-sz, SEEK_CUR);
                return ODD;
            }
        }
        ++i;
    }
    return 0;
}

void parse_CDFH(FILE* fin) {

}

#define src_len 1024
unsigned char src[src_len];
#define dst_len 1024
unsigned char dst[dst_len];
/**
 * parse header information, and save files if save_file is non-zero
 */
void parse_LFH(FILE* fin, int save_file) {
    
}

void parse_EOCD(FILE* fin) {

}

void parse_ODD(FILE* fin) {

}
