#ifndef ZIPWALK_H__
#define ZIPWALK_H__

#define TAR_FILE 1
#define GZIP_FILE 2
#define ZIP_FILE 3

#define MAX_PATH_LEN 256

int get_filetype(const char *file_path);

/**
 * mkdir a/b/c/d in current working directory
 * @param path the path to mkdir
 * @param mode 0777
 * @returns 0 on success
 */
int my_mkdir(const char *path, const mode_t mode);

/**
 * make basedir of path
 * @returns 0 on success
 */
int mkbdir(const char *path, const mode_t mode);

int getbyte(FILE *fin, u_int8_t *c);

int getword(FILE *fin, u_int16_t *w);

int getdword(FILE* fin, u_int32_t *dw);

int getqword(FILE* fin, u_int64_t *qw);

int getstr(FILE* fin, char *filename, int sz);

/**
 * @returns number of deflate bytes got inflated
 */
unsigned long my_inflate(FILE* fin, FILE* fout, unsigned char *src, unsigned long src_len, unsigned char *dst, unsigned long dst_len);

unsigned long copy_n(FILE* fin, FILE* fout, unsigned long len);

#define CDFH 1  // Central directory file header 
#define LFH 3   // Local file header
#define EOCD 5  // End of central directory
#define ODD 7   // Optional data descriptor
/**
 * @returns 0 if no headers are found, or one of CDFH, LFH, EOCD, ODD.
 * and the position indicator are set to the beginning of these signatures.
 */
int next_header(FILE* fin);

void parse_CDFH(FILE* fin);

void parse_LFH(FILE* fin, int save_file);

void parse_EOCD(FILE* fin);

void parse_ODD(FILE* fin);

int parse_GZIP_header(FILE* fin, int save_file);

void parse_GZIP_footer(FILE* fin);

#endif /* ZIPWALK_H__ */