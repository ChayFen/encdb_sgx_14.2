//
// Created by 甜甜 on 2020/12/2.
//


#include "LRUCache.hpp"

#ifdef ENCLAVE

#include "encryption.h"
extern "C" {
#include "aes/taes.h"
}
// #if !defined(ENCLAVE)
//     #include "LogBase.h"
// #endif

#include <string.h>
#include <memory>
#include "t.h"
#include <assert.h>
using namespace std;
// using namespace util;
// 根据数据字节数 获取加密字节数

int encryption::get_crypt_size(int data_len)
{
    return ((data_len / AES_BLOCKLEN) + ((data_len % AES_BLOCKLEN) == 0 ? 0 : 1)) * AES_BLOCKLEN;
}

// 加密
int encryption::aes_encrypt(char *in, char *key, char *out, size_t inlen) //, int olen)
{
    if (NULL == in || NULL == key || NULL == out)
        return -1;


    // if(inlen % AES_BLOCKLEN != 0) {
    //     Log("aes encrypt: in size % AES_BLOCK != 0", log::error)
    // }
    
    // assert(inlen % AES_BLOCKLEN == 0);

    struct AES_ctx ctx;
    AES_init_ctx(&ctx, (const uint8_t *)key);
    int en_len = 0;

    memcpy_s(out, inlen, in, inlen);
    while (en_len < inlen) //输入输出字符串够长。而且是AES_BLOCK_SIZE的整数倍，须要严格限制
    {
        AES_ECB_encrypt(&ctx, (unsigned char *)out);
        out += AES_BLOCKLEN;
        en_len += AES_BLOCKLEN;
    }
    return get_crypt_size(inlen);
}

// 解密
int encryption::aes_decrypt(char *in, int inlen, char *key, char *out)
{
    if (NULL == in || NULL == key || NULL == out)
        return -1;
    // if(inlen % AES_BLOCKLEN != 0) {
    //     Log("aes decrypt: in size % AES_BLOCK != 0", log::error)
    // }
    // assert(inlen % AES_BLOCKLEN == 0);
    struct AES_ctx ctx;
    AES_init_ctx(&ctx, (const uint8_t *)key);
    int en_len = 0;

    memcpy_s(out, inlen, in, inlen);
    while (en_len < inlen) //输入输出字符串够长。而且是AES_BLOCK_SIZE的整数倍，须要严格限制
    {
        AES_ECB_decrypt(&ctx, (unsigned char *)out);
        out += AES_BLOCKLEN;
        en_len += AES_BLOCKLEN;
    }
    return get_crypt_size(inlen);
}

long long encryption::charToLongLong(char b[])
{

    return *((long long *)b);
}

//! not used
char *encryption::DET_encrypt(char *m, size_t *cipher_length)
{
}

char *encryption::DET_decrypt(char *ciphertext, std::size_t *cipher_len)
{
}

#else

#include "encryption.h"
#include <stdio.h>
#include <fstream>
#include <ostream>
#include <string.h>
#include <memory>
#include <openssl/aes.h>
#include <stdlib.h>
#include "t.h"

using namespace std;

#ifdef USE_LRU
LRUCache<long, std::string> AESCache;
#endif
// 根据数据字节数 获取加密字节数
int encryption::get_crypt_size(int data_len)
{
    return ((data_len / AES_BLOCK_SIZE) + ((data_len % AES_BLOCK_SIZE) == 0 ? 0 : 1)) * AES_BLOCK_SIZE;
}

// 加密
int encryption::aes_encrypt(char *in, char *key, char *out, size_t inlen) //, int olen)
{
    if (NULL == in || NULL == key || NULL == out)
        return -1;

    AES_KEY aes;
    if (AES_set_encrypt_key((unsigned char *)key, 128, &aes) < 0)
    {
        return -2;
    }

    //int len=strlen(in);
    int en_len = 0;
    while (en_len < inlen) //输入输出字符串够长。而且是AES_BLOCK_SIZE的整数倍，须要严格限制
    {
        AES_encrypt((unsigned char *)in, (unsigned char *)out, &aes);
        in += AES_BLOCK_SIZE;
        out += AES_BLOCK_SIZE;
        en_len += AES_BLOCK_SIZE;
    }

    return get_crypt_size(inlen);
}

// 解密
int encryption::aes_decrypt(char *in, int in_len, char *key, char *out)
{
#ifdef USE_LRU
    string instr(in, in_len);
    auto hashv = hash<string>()(instr);
    auto tmpstr = AESCache.get(hashv, "");

    if(!tmpstr.empty()) {
        memcpy(out, tmpstr.c_str(), tmpstr.size());
        return tmpstr.size();
    }

    char* outbase = out; 
#endif
    if (NULL == in || NULL == key || NULL == out)
        return -1;

    AES_KEY aes;
    if (AES_set_decrypt_key((unsigned char *)key, 128, &aes) < 0)
    {
        return -2;
    }

    int en_len = 0;
    while (en_len < in_len)
    {
        AES_decrypt((unsigned char *)in, (unsigned char *)out, &aes);
        in += AES_BLOCK_SIZE;
        out += AES_BLOCK_SIZE;
        en_len += AES_BLOCK_SIZE;
    }
#ifdef USE_LRU
    AESCache.put(hashv, string(outbase, en_len));
#endif
    return en_len;
}

long long encryption::charToLongLong(char b[])
{
    // long long result=0L;
    // for(int i=0;i<sizeof(long long);i++)
    // {
    //     result<<=
    // }
    return *((long long *)b);
}
//! not used
char *encryption::DET_encrypt(char *m, size_t *cipher_length)
{
    //encryption encrypt;
    t change;
    char buf[100];
    memset(buf, 1, sizeof(buf));
    strcpy((char *)buf, m);
    size_t inlen;
    //std::// plain value is :" << buf << std::endl;

    inlen = strlen(buf);

    char aes_keybuf[32];
    memset(aes_keybuf, 0, sizeof(aes_keybuf));

    ifstream infile;
    infile.open("SAHE_key.txt");
    if (!infile.is_open())
        abort();
    while (!infile.eof())
    {
        infile.getline(aes_keybuf, 32);
    }
    infile.close();

    char *aes_en = new char[BUF_SIZE]();
    //char aes_en[BUF_SIZE] = {0};

    int aes_en_len = aes_encrypt(buf, aes_keybuf, aes_en, inlen);
    *cipher_length = aes_en_len;

    // aes-128 加密
    //    if (aes_en_len < 0)
    //    {
    //        printf("aes_encrypt error ret: %d\n", aes_en_len);
    //        return -1;
    //    }

    // size_t bufsz1 = strlen(aes_en) + 1;
    // size_t hexsz1 = 2 * (strlen(aes_en) + 1);
    // char* h = new char[hexsz1 * 2];
    // change.buf2hex((uint8_t*)aes_en, strlen(aes_en), h, &hexsz1);

    return aes_en;
}

char *encryption::DET_decrypt(char *ciphertext, std::size_t *cipher_len)

{
    encryption encrypt;
    t change;
    // size_t bufsz2 = sizeof(ciphertext) + 1;
    // size_t hexsz2 = 2 * (strlen(ciphertext) + 1);
    // char* nc2 = new char[hexsz2];
    // change.hex2buf((uint8_t*)nc2, &hexsz2, ciphertext, hexsz2);
    // cout<<"cipher1_parse: "<<nc2<<endl;
    char *aes_de = new char[BUF_SIZE]();
    //char aes_de[BUF_SIZE] = {0};
    char aes_keybuf[32];
    memset(aes_keybuf, 0, sizeof(aes_keybuf));
    //memset(aes_keybuf, 0, sizeof(aes_keybuf));

    ifstream infile2;
    infile2.open("SAHE_key.txt");
    if (!infile2.is_open())
        // SAHE_KEY file failure" << endl;
        while (!infile2.eof())
        {
            infile2.getline(aes_keybuf, 32);
        }
    infile2.close();

    //strcpy((char*)aes_keybuf, CRYPT_USER_KEY);

    //const   char*   p2   =   (const   char*)(char*)aes_de;
    int aes_de_len = encrypt.aes_decrypt(ciphertext, strlen(ciphertext), aes_keybuf, aes_de); // aes-128 解密
    *cipher_len = aes_de_len;

    // printf("aes_de_len: %d aes_de: %s\n", strlen(aes_de), aes_de);
    // " << strlen(aes_de) << endl;

    return aes_de;
}

#endif
