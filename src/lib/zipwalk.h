#ifndef ZIPWALK_H__
#define ZIPWALK_H__

/**
 * @returns number of deflate bytes got inflated
 */
unsigned long my_inflate(FILE* fin, FILE* fout, unsigned char *src, unsigned long src_len, unsigned char *dst, unsigned long dst_len);

#endif /* ZIPWALK_H__ */