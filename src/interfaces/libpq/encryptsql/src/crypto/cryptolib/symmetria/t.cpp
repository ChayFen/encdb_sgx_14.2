//
// Created by 甜甜 on 2020/12/16.
//

#include "t.h"

#include <string>
#include <stdlib.h>

#ifndef ENCLAVE
#include <iostream>
#include <sstream>
#include <iomanip>
#endif

using namespace std;

int t::buf2hex(const uint8_t *buf, size_t bufSize, char *hexstr, size_t *hexstrlen)
{ // need to alloc hexstr by yourself

#ifdef ENCLAVE
    if (*hexstrlen < bufSize * 2)
        return -1;

    constexpr char high = 0xf0; // 1111 0000;
    constexpr char low = 0x0f;  // 0000 1111;
    int cnt = 0;
    for (int i = 0; i < bufSize; i++)
    {
        char h = (buf[i] & high) >> 4;
        char l = buf[i] & low;

        h = h >= 10 ? h + 97 : h + 48;
        l = l >= 10 ? l + 97 : l + 48;
        hexstr[cnt++] = h;
        hexstr[cnt++] = l;
    }
    *hexstrlen = cnt;
    return 0;
#else

    if (*hexstrlen < bufSize * 2)
        return -1;

    std::stringstream ss;

    for (int i = 0; i < bufSize; ++i)
        ss << std::setfill('0') << std::setw(2) << std::hex << (0xff & (unsigned char)buf[i]);
    string mystr = ss.str() + '\0';
    strcpy(hexstr, mystr.c_str());
    *hexstrlen = mystr.size();
    return 0;
#endif
}

int t::hex2buf(uint8_t *buf, size_t *bufSize, const char *strhex, size_t hexmystrlen) //  // need to alloc buf by yourself
{
    if (*bufSize < hexmystrlen / 2)
        return -1;

    for (unsigned int i = 0; i < hexmystrlen; i += 2)
    {
        string byteString(strhex + i, strhex + i + 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        buf[i / 2] = byte;
    }

    *bufSize = hexmystrlen / 2;

    return 0;
}

void mymemcpy(void *dest, const void *src, size_t size)
{

#ifdef ENCLAVE
    memcpy_s(dest, size, src, size);
#else
    memcpy(dest, src, size);
#endif
}
