#ifndef __PUBLIC_H__
#define __PUBLIC_H__
extern void dispBuf(unsigned char *buf, int len, const char *name);
extern int bcd_to_string(char *bcd, char *strDst, int size);
extern int string_to_bcd(char *str, char* bcdDst, int size);


#endif


