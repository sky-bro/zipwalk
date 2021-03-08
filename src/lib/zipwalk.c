#include <stdio.h>
#include <zlib.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "zipwalk.h"

int my_mkdir(const char *path, const mode_t mode) { // path modifiable
    if (!path || !path[0]) {
        fprintf(stderr, "[-] <null> path detected!\n");
        return 1;
    }
    if (path[0] == '/') {
        fprintf(stderr, "[-] path cannot start with '/'\n");
        return 1;
    }
    int len = strnlen(path, MAX_PATH_LEN);
    if (len > MAX_PATH_LEN) {
        fprintf(stderr, "[-] path is too long, MAX_PATH_LEN: %d\n", MAX_PATH_LEN);
        return 1;
    }
    char tmp[MAX_PATH_LEN+2]; // "/\0"
    strncpy(tmp, path, MAX_PATH_LEN);
    if (tmp[len-1] != '/') tmp[len] = '/', tmp[len+1] = 0;
    else tmp[len] = 0;
    struct stat s;
    for (char *p = tmp; *p; ++p) {
        if (*p == '/') {
            *p = 0;
            if (stat(path, &s)) { // not exist
                if (mkdir(path, mode) < 0) {
                    perror("[-] mkdir error");
                    return 1;
                }
            } else if (!S_ISDIR(s.st_mode)) { // exist, but not directory
                fprintf(stderr, "[-] %s is ont a folder!\n", tmp);
                return 1;
            }
            *p = '/';
        }
    }
    return 0;
}

int mkbdir(const char *path, const mode_t mode) {
    if (!path || path[0] == 0) {
        fprintf(stderr, "[-] <null> path detected!\n");
        return 1;
    }
    if (path[0] == '/') {
        fprintf(stderr, "[-] path cannot start with '/'\n");
        return 1;
    }
    char *p = strrchr(path, '/');
    if (!p) {
        // in current folder, no need to mkdir
        return 0;
    }
    char tmp[MAX_PATH_LEN];
    int len = p - path + 1;
    if (len >= MAX_PATH_LEN) {
        fprintf(stderr, "[-] path is too long, MAX_PATH_LEN: %d\n", MAX_PATH_LEN);
        return 1;
    }
    strncpy(tmp, path, len);
    tmp[len] = 0;
    return my_mkdir(tmp, mode);
}

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
    printf("[*] %lu bytes inflated to %lu bytes\n", len_in, len_out);
    (void)inflateEnd(&strm);
    fseek(fin, -(len_in + strm.avail_in), SEEK_CUR);
    return len_in;
}

unsigned long copy_n(FILE* fin, FILE* fout, unsigned long len) {
    char c = fgetc(fin);
    unsigned long ret = len;
    while (len--) {
        if (c == EOF) {
            fprintf(stderr, "EOF detected, %lu byte(s) not copyed\n", len);
            break;
        }
        fputc(c, fout);
        fgetc(fin);
    }
    return ret - len - 1;
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
    u_int32_t dw;
    u_int16_t w, method, name_len, extra_len, comment_len;
    char *filename = NULL;
    long pos = ftell(fin);
    if (getdword(fin, &dw)) return;
    printf("[+] Central Directory File Header (%#X) at offset %#X:\n", dw, pos);
    if (getword(fin, &w)) return;
    printf("\tVersion made by: %#X\n", w);
    if (getword(fin, &w)) return;
    printf("\tVersion needed to extract (minimum): %#X\n", w);
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
    printf("\tExtra field length: %d\n", extra_len);
    if (getword(fin, &comment_len)) return;
    printf("\tComment length: %d\n", comment_len);
    if (getword(fin, &w)) return;
    printf("\tDisk number where file starts: %d\n", w);
    if (getword(fin, &w)) return;
    printf("\tInternal file attributes: %#X\n", w);
    if (getdword(fin, &dw)) return;
    printf("\tExternal file attributes: %#X\n", dw);
    if (getdword(fin, &dw)) return;
    printf("\tRelative offset of local file header: %#X\n", dw);
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
    free(filename);
    filename = NULL;
    // ignore extra filed for now, skip extra_len bytes
    fseek(fin, extra_len, SEEK_CUR);
    printf("\tExtra field: %d byte(s) skipped\n", extra_len);
    if (comment_len == 0) return;
    char *comment = malloc(comment_len+1);
    sz = fread(comment, sizeof(char), comment_len, fin);
    if (sz == comment_len) {
        comment[comment_len] = 0;
        printf("\tComment: %s\n", comment);
    }
    free(comment);
    comment = NULL;
}

#define src_len 1024
unsigned char src[src_len];
#define dst_len 1024
unsigned char dst[dst_len];
/**
 * parse header information, and save files if save_file is non-zero
 */
void parse_LFH(FILE* fin, int save_file) {
    u_int32_t dw, compressed_len, uncompressed_len;
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
    if (getdword(fin, &compressed_len)) return;
    printf("\tCompressed size: %u byte(s)\n", compressed_len);
    if (getdword(fin, &uncompressed_len)) return;
    printf("\tUncompressed size: %u byte(s)\n", uncompressed_len);
    if (getword(fin, &name_len)) return;
    printf("\tFile name length: %d\n", name_len);
    if (getword(fin, &extra_len)) return;
    printf("\tExtra field length: %d\n", extra_len);
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
        printf("[*] creating %s\n", filename);
        if (filename[name_len-1] == '/') { // is a folder
            my_mkdir(filename, 0777);
            save_file = 0; // no save file for a folder
        } else {
            mkbdir(filename, 0777);
            fout = fopen(filename, "wb");
            if (!fout) {
                // will fail when folders not created
                perror("[-] fail to open file");
                free(filename);
                filename = NULL;
                return;
            }
        }
    }
    free(filename);
    filename = NULL;
    // ignore extra filed for now, skip extra_len bytes
    fseek(fin, extra_len, SEEK_CUR);
    printf("\tExtra field: %d byte(s) skipped\n", extra_len);
    if (save_file) {
        if (method == 8) // deflate
            my_inflate(fin, fout, src, src_len, dst, dst_len);
        else if (method == 0) { // stored
            unsigned long pos = ftell(fin);
            printf("[*] %lu of %lu byte(s) stored\n", copy_n(fin, fout, compressed_len), compressed_len);
            fseek(fin, pos, SEEK_SET);
        } else {
            printf("[-] method [%d] is not supported yet\n", method);
        }
        fclose(fout);
    } else {
        // do nothing...
        // or skip filecontent?
    }
}

void parse_EOCD(FILE* fin) {
    u_int32_t dw;
    u_int16_t w, comment_len;
    long pos = ftell(fin);
    if (getdword(fin, &dw)) return;
    printf("[+] End of Central Directory Record (%#X) at offset %#X:\n", dw, pos);
    if (getword(fin, &w)) return;
    printf("\tNumber of this disk: %d\n", w);
    if (getword(fin, &w)) return;
    printf("\tDisk where central directory starts: %d\n", w);
    if (getword(fin, &w)) return;
    printf("\tNumber of central directory records on this disk: %d\n", w);
    if (getword(fin, &w)) return;
    printf("\tTotal number of central directory records: %d\n", w);
    if (getdword(fin, &dw)) return;
    printf("\tSize of central directory (bytes): %d\n", dw);
    if (getdword(fin, &dw)) return;
    printf("\tOffset of start of central directory: %#X\n", dw);
    if (getword(fin, &comment_len)) return;
    printf("\tComment length: %d\n", comment_len);
    if (comment_len == 0) return;
    char *comment = malloc(comment_len+1);
    int sz = fread(comment, sizeof(char), comment_len, fin);
    if (sz == comment_len) {
        comment[comment_len] = 0;
        printf("\tComment: %s\n", comment);
    }
    free(comment);
    comment = NULL;
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
