#ifndef ZIPWALK_H__
#define ZIPWALK_H__

int getbyte(FILE *fin, u_int8_t *c);

int getword(FILE *fin, u_int16_t *w);

int getdword(FILE* fin, u_int32_t *dw);

int getqword(FILE* fin, u_int64_t *qw);

/**
 * @returns number of deflate bytes got inflated
 */
unsigned long my_inflate(FILE* fin, FILE* fout, unsigned char *src, unsigned long src_len, unsigned char *dst, unsigned long dst_len);

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

#endif /* ZIPWALK_H__ */