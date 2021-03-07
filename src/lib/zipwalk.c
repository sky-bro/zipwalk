#include <stdio.h>
#include <zlib.h>
#include <stdlib.h>
#include "zipwalk.h"

int getbyte(FILE *fin, u_int8_t *c) {;
    if (fread(c, 1, 1, fin)) {
        return 0;
    }
    return 1;
}

int getword(FILE *fin, u_int16_t *w) {
    u_int8_t c;
    if (getbyte(fin, &c)) return 1;
    *w = c;
    if (getbyte(fin, &c)) return 1;
    *w |= (u_int16_t)(c) << 8;
    return 0;
}

int getdword(FILE* fin, u_int32_t *dw) {
    u_int16_t w;
    if (getword(fin, &w)) return 1;
    *dw = w;
    if (getword(fin, &w)) return 1;
    *dw |= (u_int32_t)(w) << 16;
    return 0;
}

int getqword(FILE* fin, u_int64_t *qw) {
    u_int32_t dw;
    if (getdword(fin, &dw)) return 1;
    *qw = dw;
    if (getdword(fin, &dw)) return 1;
    *qw |= (u_int64_t)(dw) << 32;
    return 0;
}

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
        // printf("[*] len_in: %lu, len_out: %lu\n", len_in, len_out);
        fwrite(dst, sizeof(unsigned char), dst_len - strm.avail_out, fout);
        if (ret != Z_OK) {
            break;
        }
    }
    printf("[+] %lu bytes inflated to %lu bytes\n", len_in, len_out);
    (void)inflateEnd(&strm);
    fseek(fin, -(len_in + strm.avail_in), SEEK_CUR);
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
    u_int32_t dw;
    u_int16_t w, method, name_len, extra_len;
    char *filename = NULL;
    FILE *fout = NULL;
    long pos = ftell(fin);
    if (getdword(fin, &dw)) return;
    printf("[+] Local File Header (%#X) at offset %#X:\n", dw, pos);
    if (getword(fin, &w)) return;
    printf("\tversion to extract (minimum): %d\n", w);
    if (getword(fin, &w)) return;
    printf("\tgeneral purpose bit flag: %#X\n", w);
    if (getword(fin, &method)) return;
    printf("\tcompression method: %d\n", method);
    // TODO: human readable time & date, set file time & date
    if (getword(fin, &w)) return;
    printf("\tFile last modification time: %d\n", w);
    if (getword(fin, &w)) return;
    printf("\tFile last modification date: %d\n", w);
    if (getdword(fin, &dw)) return;
    printf("\tCRC-32 of uncompressed data: %#X\n", dw);
    if (getdword(fin, &dw)) return;
    printf("\tCompressed size: %u byte(s)\n", dw);
    if (getdword(fin, &dw)) return;
    printf("\tUncompressed size: %u byte(s)\n", dw);
    if (getword(fin, &name_len)) return;
    printf("\tFile name length: %d\n", name_len);
    if (getword(fin, &extra_len)) return;
    printf("\tExtra field length: %d\n", name_len);
    if (name_len == 0) return;
    // ignore extra field for now
    filename = malloc(name_len+1);
    int sz = fread(filename, sizeof(char), name_len, fin);
    if (sz != name_len || filename[0] == '/') { // filename cannot start with '/'
        free(filename);
        filename = NULL;
        return;
    }
    filename[name_len] = 0;
    printf("\tFile name: %s\n", filename);
    // if extract files
    if (save_file) {
        printf("[+] creating %s\n", filename);
        if (filename[name_len-1] == '/') { // is a folder
            save_file = 0; // no save file for a folder
        } else {
            fout = fopen(filename, "wb");
            free(filename);
            filename = NULL;
            if (!fout) {
                // will fail when folders not created
                perror("[-] fail to open file");
                return;
            }
        }
    } else {
        free(filename);
        filename = NULL;
    }
    // ignore extra filed for now, skip extra_len bytes
    fseek(fin, extra_len, SEEK_CUR);
    printf("\tExtra field: %d byte(s) skipped\n", extra_len);
    if (save_file) {
        if (method == 8) // deflate
            my_inflate(fin, fout, src, src_len, dst, dst_len);
        fclose(fout);
    } else {
        // do nothing...
        // or skip filecontent?
    }
}

void parse_EOCD(FILE* fin) {
    u_int32_t dw;

}

void parse_ODD(FILE* fin) {
    u_int32_t dw;
    long pos = ftell(fin);
    if (getdword(fin, &dw)) return;
    printf("[+] Optional Data Descriptor (%#X) at offset %#X:\n", dw, pos);
    if (getdword(fin, &dw)) return;
    printf("\tCRC-32 of uncompressed data: %#X\n", dw);
    if (getdword(fin, &dw)) return;
    printf("\tCompressed size: %u byte(s)\n", dw);
    if (getdword(fin, &dw)) return;
    printf("\tUncompressed size: %u byte(s)\n", dw);
}
