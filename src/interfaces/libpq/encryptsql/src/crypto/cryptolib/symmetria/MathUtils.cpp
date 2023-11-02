#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <climits>

#include "MathUtils.h"

#ifdef ENCLAVE
#include <sgx_trts.h>

#else
#include <time.h>
#include <math.h>
#endif


using namespace std;
#define N 999

/**
     * Return a random long long  number in the range -range/2 to range/2
     */

long long MathUtils::randLong(long long range)
{


    float random;

#ifdef ENCLAVE
    unsigned int rand;
    sgx_read_rand((uint8_t *)&rand, sizeof(rand));
    random = rand % (N + 1) / (float)(N + 1);

#else
    srand(time(NULL)); //设置随机数种子，使每次产生的随机序列不同
    random = rand() % (N + 1) / (float)(N + 1);
#endif


    return (long long)(random * range - range / 2);
}
/**
     * Return a random long long  number in the range 0 to max
     */
long long MathUtils::randLongPos(long long max)
{
    float random;
#ifdef ENCLAVE
    unsigned int rand;
    sgx_read_rand((uint8_t *)&rand, sizeof(rand));
    random = rand % (N + 1) / (float)(N + 1);

#else
    srand(time(NULL)); //设置随机数种子，使每次产生的随机序列不同
    random = rand() % (N + 1) / (float)(N + 1);
#endif
    return (long long)(random * max);
}

long long MathUtils::gcd(long long a, long long b)
{
    return b == 0 ? (a < 0 ? -a : a) : gcd(b, a % b);
}
//////////////////////////////////////////////////////////////////////////////
// public static long long  gcdBI(long long  a, long long  b) {
//     return BigInteger.valueOf(a).gcd(BigInteger.valueOf(b)).long long Value();
// }

/**
     * This function performs the extended euclidean algorithm on two numbers a and b. The function
     * returns the gcd(a,b) as well as the numbers x and y such that ax + by = gcd(a,b). This
     * calculation is important in number theory and can be used for several things such as finding
     * modular inverses and solutions to linear Diophantine equations.
     */
long long *MathUtils::egcd(long long a, long long b)
{
    if (b == 0)
    {
        //vector<long long > array;
        //long long array[3];
        long long *array = new long long[3];
        if (a < 0)
        {
            array[0] = -a;
            array[1] = 0L;
            array[2] = 0L;
        }
        else
        {
            array[0] = a;
            array[1] = 1L;
            array[2] = 0L;
        }

        return array;
    }
    long long *v = egcd(b, a % b);
    long long tmp = v[1] - v[2] * (a / b);
    v[1] = v[2];
    v[2] = tmp;
    return v;
}

long long MathUtils::mod(long long a, long long modulo)
{
    long long r = a % modulo;
    if (r < 0)
    {
        r += modulo;
    }
    return r;
}

// public static long long  modBI(long long  a, long long  modulo) {
//     return BigInteger.valueOf(a).mod(BigInteger.valueOf(modulo)).long long Value();
// }

long long MathUtils::modAdd(long long a, long long b, long long modulo)
{
    //unsigned long long bignum=ULLONG_MAX+1;
    a = mod(a, modulo);
    b = mod(b, modulo);
    long long r = a + b;
    if (r < 0)
        r += 2;
    return mod(r, modulo);
}
/////////////////////////////////////////////////////////////////////////////
// public static long long  modAddBI(long long  a, long long  b, long long  modulo) {
//     return BigInteger.valueOf(a).add(BigInteger.valueOf(b)).mod(BigInteger.valueOf(modulo)).long long Value();
// }

long long MathUtils::modSubtract(long long a, long long b, long long modulo)
{
    return modAdd(a, -b, modulo);
}

// public static long long  modSubtractBI(long long  a, long long  b, long long  modulo) {
//     return BigInteger.valueOf(a).subtract(BigInteger.valueOf(b)).mod(BigInteger.valueOf(modulo)).long long Value();
// }

long long MathUtils::modNegate(long long a, long long modulo)
{
    return mod(-a, modulo);
}

// public static long long  modNegateBI(long long  a, long long  modulo) {
//     return BigInteger.valueOf(a).negate().mod(BigInteger.valueOf(modulo)).long long Value();
// }

/**
     * source: https://stackoverflow.com/questions/12168348/ways-to-do-modulo-multiplication-with-primitive-types
     */
long long MathUtils::modMul(long long a, long long b, long long modulo)
{

    if (a == 1)
        return b;
    if (b == 1)
        return a;

    a = mod(a, modulo);
    b = mod(b, modulo);

    if (a == 1)
        return b;
    if (b == 1)
        return a;

    long long res = 0;
    long long temp_b;

    while (a != 0)
    {
        if ((a & 1) == 1)
        {
            // Add b to res, n m, without overflow
            if (b >= modulo - res) // Equiv to if (res + b >= m), without overflow
                res -= modulo;
            res += b;
        }
        a >>= 1;

        // Double b, n m
        temp_b = b;
        if (b >= modulo - b) // Equiv to if (2 * b >= m), without overflow
            temp_b -= modulo;
        b += temp_b;
    }
    return res;
}

long long MathUtils::modDiv(long long a, long long b, long long modulo)
{
    return modMul(a, modInverse(b, modulo), modulo);
}
////////////////////////////////////////////////////////////////////////////
// public static long long  modMulBI(long long  a, long long  b, long long  modulo) {
//     return BigInteger.valueOf(a).multiply(BigInteger.valueOf(b)).mod(BigInteger.valueOf(modulo)).long long Value();
// }

long long MathUtils::modPow(long long a, long long b, long long modulo)
{
    //return modPowBI(a, b, modulo); // modPowBI faster
    if (modulo <= 0)

        throw "modulo is less than 0";

    bool invertResult;
    if ((invertResult = (b < 0)))
        b = modNegate(b, modulo);

    a = mod(a, modulo);
    if (a == 0)
        return 0;
    long long res = 1;
    while (b > 0)
    {
        if (b % 2 == 1)
            res = modMul(res, a, modulo);
        a = modMul(a, a, modulo);
        b >>= 1;
    }

    if (invertResult)
    {
        if (gcd(res, modulo) != 1)
        {

            return 0;
        }
        res = modInverse(res, modulo);
    }

    return res;
}

long long MathUtils::modInverse(long long a, long long modulo)
{
    a = modAdd(a, modulo, modulo);
    long long *v = egcd(a, modulo);
    long long x = v[1];
    delete[] v;
    return modAdd(x, modulo, modulo);
}
