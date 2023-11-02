//
// Created by 甜甜 on 2020/12/16.
//

#ifndef SYMMETRIA_2_T_H
#define SYMMETRIA_2_T_H

#ifdef ENCLAVE
#include <tlibc/mbusafecrt.h>
#else
#include <string.h>
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned long size_t;

void mymemcpy(void *dest, const void *src, size_t size);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class t
{
public:
    int buf2hex(const uint8_t *buf, size_t bufSize, char *hexstr, size_t *hexmystrlen);
    int hex2buf(uint8_t *buf, size_t *bufSize, const char *strhex, size_t hexmystrlen);
    void testbuf2hex(char *c);
    void testBufConversion();
};
#endif
#endif //SYMMETRIA_2_T_H
