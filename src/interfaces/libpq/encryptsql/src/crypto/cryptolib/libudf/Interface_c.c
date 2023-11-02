
#include "stdio.h"
#include "Interface.h"
#include <string.h>
#include "postgres.h"
#include "fmgr.h"
#include "utils/geo_decls.h"
#include "ORE_STR22STRUCT.h"
#include <unistd.h>
#include <assert.h>
#include "stdlib.h"
#include "udfDispatch.h"
#include "t.h"
#include "Timer.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

extern const char *getFloatScaleCiphertext(); //config.h
extern void c_count(const char* name, double time); //TimeCounter.h
// 两个text以\0分割
text *
concat_text(text *arg1, text *arg2)
{
   int32 arg1_size = VARSIZE(arg1) - VARHDRSZ;
   int32 arg2_size = VARSIZE(arg2) - VARHDRSZ;
   int32 new_text_size = arg1_size + arg2_size + VARHDRSZ + 1;
   text *new_text = (text *)palloc(new_text_size);

   SET_VARSIZE(new_text, new_text_size);
   memcpy(VARDATA(new_text), VARDATA(arg1), arg1_size);
   VARDATA(new_text)
   [arg1_size] = '\0';
   memcpy(VARDATA(new_text) + arg1_size + 1, VARDATA(arg2), arg2_size);
   return new_text;
}

void split_to_2_text(text *src, text **ore, text **aes)
{
   int32 arg1_size = VARSIZE(src) - VARHDRSZ;
   int32 ore_size = 0;
   if (arg1_size != 0)
      ore_size = strlen(VARDATA(src)); // 初始时src不一定有\0
   int32 aes_size = Max(arg1_size - ore_size - 1, 0);

   text *pore = (text *)palloc(ore_size + VARHDRSZ);
   SET_VARSIZE(pore, ore_size + VARHDRSZ);

   text *paes = (text *)palloc(aes_size + VARHDRSZ);
   SET_VARSIZE(paes, aes_size + VARHDRSZ);

   memcpy(VARDATA(paes), VARDATA(src) + ore_size + 1, aes_size);
   memcpy(VARDATA(pore), VARDATA(src), ore_size);

   *ore = pore;
   *aes = paes;
}

PG_FUNCTION_INFO_V1(sahe_add);
Datum sahe_add(PG_FUNCTION_ARGS)
{
   void* timer = startTimer();
   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   text *arg3 = PG_GETARG_TEXT_P(2);
   text *arg4 = PG_GETARG_TEXT_P(3);

   //! BUG arg1->vl_dat可能不为 \0 也就没有c风格字符串
   // TODO：此为低效解决方案
   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;
   size_t s3 = VARSIZE(arg3) - VARHDRSZ;
   size_t s4 = VARSIZE(arg4) - VARHDRSZ;

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   memcpy(dat3, arg3->vl_dat, s3);
   memcpy(dat4, arg4->vl_dat, s4);

   bool isLeftFloat = dat1[s1 - 1] == '2';
   bool isRightFloat = dat2[s2 - 1] == '2';
   // 去除标志位
   dat1[s1 - 1] = 0;
   dat2[s2 - 1] = 0;

   if (isLeftFloat || isRightFloat)
   {
      if (!(isLeftFloat && isRightFloat))
      {
         if (!isLeftFloat)
         {
            //TODO: 怎么乘倍数？
         }
      }
   }
   char *buffer = saheAdd(dat1, dat2, dat3, dat4);

   size_t buffSize = strlen(buffer);
   text *destination = (text *)palloc0(VARHDRSZ + buffSize + 1);
   SET_VARSIZE(destination, VARHDRSZ + buffSize + 1);

   char *pbuf = VARDATA(destination);
   memcpy(pbuf, buffer, buffSize);
   free(buffer);
   // 没有解决加法下放缩的问题，只能先设置为1了
   destination->vl_dat[buffSize] = '1';
c_count("interface_c: ahe", getPassedTime(timer, true));
   PG_RETURN_TEXT_P(destination);
}

PG_FUNCTION_INFO_V1(sahe_addplain);

Datum
    sahe_addplain(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);
   int32 m1 = PG_GETARG_INT32(1);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   // size_t s2 = VARSIZE(arg2) - VARHDRSZ;

   char *dat1 = palloc(s1 + 1);
   // char *dat2 = palloc(s2 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   // memcpy(dat2, arg2->vl_dat, s2);
   dat1[s1] = '\0';
   // 去除标志位
   dat1[s1 - 1] = 0;

   char *buffer = saheAddp(dat1, m1); /* our source data */

   size_t buffSize = strlen(buffer);

   text *destination = (text *)palloc0(VARHDRSZ + buffSize + 1);
   SET_VARSIZE(destination, VARHDRSZ + buffSize + 1);
   memcpy(destination->vl_dat, buffer, buffSize);
// 没有解决加法下放缩的问题，只能先设置为1了
destination->vl_dat[buffSize ] = '1';
   free(buffer);

   free(dat1);
   PG_RETURN_TEXT_P(destination);
}

PG_FUNCTION_INFO_V1(sahe_substract);

Datum
    sahe_substract(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   text *arg3 = PG_GETARG_TEXT_P(2);
   text *arg4 = PG_GETARG_TEXT_P(3);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;
   size_t s3 = VARSIZE(arg3) - VARHDRSZ;
   size_t s4 = VARSIZE(arg4) - VARHDRSZ;

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   memcpy(dat3, arg3->vl_dat, s3);
   memcpy(dat4, arg4->vl_dat, s4);

   // 去除标志位
   dat1[s1 - 1] = 0;
   dat2[s2 - 1] = 0;

   char *buffer = saheSub(dat1, dat2, dat3, dat4); /* our source data */
   size_t buffSize = strlen(buffer);

   text *destination = (text *)palloc(VARHDRSZ + buffSize + 1);
   SET_VARSIZE(destination, VARHDRSZ + buffSize + 1);
   mymemcpy(destination->vl_dat, buffer, buffSize);

   destination->vl_dat[buffSize ] = '1';
   free(buffer);
// 没有解决加法下放缩的问题，只能先设置为1了

   PG_RETURN_TEXT_P(destination);
}

PG_FUNCTION_INFO_V1(sahe_multiply);

Datum
    sahe_multiply(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);
   int32 m1 = PG_GETARG_INT32(1);
   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   // size_t s2 = VARSIZE(arg2) - VARHDRSZ;

   char *dat1 = palloc(s1 + 1);
   // char *dat2 = palloc(s2 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   // memcpy(dat2, arg2->vl_dat, s2);
   dat1[s1] = '\0';
   // dat2[s2] = '\0';
   char *buffer = saheMul(dat1, m1); /* our source data */

   size_t buffSize = strlen(buffer);
   text *destination = (text *)palloc(VARHDRSZ + buffSize + 1);
   SET_VARSIZE(destination, VARHDRSZ + buffSize + 1);
   mymemcpy(destination->vl_dat, buffer, buffSize);

   free(buffer);
   free(dat1);
   PG_RETURN_TEXT_P(destination);
}

PG_FUNCTION_INFO_V1(sahe_negate);

Datum
    sahe_negate(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   // size_t s2 = VARSIZE(arg2) - VARHDRSZ;

   char *dat1 = palloc(s1 + 1);
   // char *dat2 = palloc(s2 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   // memcpy(dat2, arg2->vl_dat, s2);
   // 去除标志位
   dat1[s1 - 1] = 0;

   char *buffer = saheNeg(dat1); /* our source data */
   size_t buffSize = strlen(buffer);
   text *destination = (text *)palloc(VARHDRSZ + buffSize + 1);
   SET_VARSIZE(destination, VARHDRSZ + buffSize + 1);
   mymemcpy(destination->vl_dat, buffer, buffSize);

   free(buffer);
   free(dat1);
// 没有解决加法下放缩的问题，只能先设置为1了
destination->vl_dat[buffSize ] = '1';
   PG_RETURN_TEXT_P(destination);
}

PG_FUNCTION_INFO_V1(smhe_multiply);

Datum
    smhe_multiply(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   text *arg3 = PG_GETARG_TEXT_P(2);
   text *arg4 = PG_GETARG_TEXT_P(3);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;
   size_t s3 = VARSIZE(arg3) - VARHDRSZ;
   size_t s4 = VARSIZE(arg4) - VARHDRSZ;

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   memcpy(dat3, arg3->vl_dat, s3);
   memcpy(dat4, arg4->vl_dat, s4);

   bool leftOpIsFloat = dat1[s1 - 1] == '2';
   bool rightOpIsFloat = dat2[s2 - 1] == '2';
   // 去掉标识位
   dat1[s1 - 1] = 0;
   dat2[s2 - 1] = 0;
   char *buffer = smheMul(dat1, dat2, dat3, dat4); /* our source data */
   size_t buffSize = strlen(buffer);
   char *buffer2 = NULL;
   // 浮点数的同台乘法是放缩处理的，两个处理后的浮点数相乘会导致缩放失真，需要除掉一个缩放倍数
   if (leftOpIsFloat && rightOpIsFloat)
   {
      char *scaleSMHECiphertext = getFloatScaleCiphertext();
      buffer2 = SMHE_DIVIDE_udf(buffer, scaleSMHECiphertext, buffSize, strlen(scaleSMHECiphertext));
   }
   else
   {
      // 两个操作数并非都是浮点数，不需要操作。
      buffer2 = buffer;
   }

   // buffer= 46(char) + 1(flag) + 1(\0)
   text *destination = (text *)palloc0(VARHDRSZ + strlen(buffer2) + 1);

   SET_VARSIZE(destination, VARHDRSZ + strlen(buffer2) + 1);
   mymemcpy(destination->vl_dat, buffer2, strlen(buffer2));
   // 添加标识位
   if (leftOpIsFloat || rightOpIsFloat)
   {
      destination->vl_dat[strlen(buffer2)] = '2';
   }
   else {
      destination->vl_dat[strlen(buffer2)] = '1';
   }

   free(buffer);
   if (buffer2 != buffer)
   {
      free(buffer2);
   }
   PG_RETURN_TEXT_P(destination);
}

PG_FUNCTION_INFO_V1(smhe_multiplyplain);

Datum
    smhe_multiplyplain(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);
   int32 m1 = PG_GETARG_INT32(1);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   // size_t s2 = VARSIZE(arg2) - VARHDRSZ;

   char *dat1 = palloc(s1 + 1);
   // char *dat2 = palloc(s2 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   // memcpy(dat2, arg2->vl_dat, s2);
   dat1[s1 - 1] = '\0';
   // dat2[s2] = '\0';

   char *buffer = smheMulp(dat1, m1); /* our source data */
   size_t buffSize = strlen(buffer);
   text *destination = (text *)palloc(VARHDRSZ + buffSize + 1);
   SET_VARSIZE(destination, VARHDRSZ + buffSize + 1);
   mymemcpy(destination->vl_dat, buffer, buffSize);

   free(buffer);
   free(dat1);
// 没有解决加法下放缩的问题，只能先设置为1了
destination->vl_dat[buffSize ] = '1';
   PG_RETURN_TEXT_P(destination);
}

PG_FUNCTION_INFO_V1(smhe_divide);

Datum
    smhe_divide(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   text *arg3 = PG_GETARG_TEXT_P(2);
   text *arg4 = PG_GETARG_TEXT_P(3);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;
   size_t s3 = VARSIZE(arg3) - VARHDRSZ;
   size_t s4 = VARSIZE(arg4) - VARHDRSZ;

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   memcpy(dat3, arg3->vl_dat, s3);
   memcpy(dat4, arg4->vl_dat, s4);

   bool leftOpIsFloat = dat1[s1 - 1] == '2';
   bool rightOpIsFloat = dat2[s2 - 1] == '2';
   // 去掉标识位
   dat1[s1 - 1] = 0;
   dat2[s1 - 1] = 0;

   char *buffer = smheDiv(dat1, dat2, dat3, dat4); /* our source data */
   size_t buffSize = strlen(buffer);
   char *buffer2 = NULL;
   // 浮点数的同台乘法是放缩处理的，两个处理后的浮点数相乘会导致缩放失真，需要除掉一个缩放倍数
   if (leftOpIsFloat && rightOpIsFloat)
   {
      char *scaleSMHECiphertext = getFloatScaleCiphertext();
      buffer2 = SMHE_MULTIPLY_udf(buffer, scaleSMHECiphertext, buffSize, strlen(scaleSMHECiphertext));
   }
   else
   {
      // 两个操作数并非都是浮点数，不需要操作。
      buffer2 = buffer;
   }

   text *destination = (text *)palloc0(VARHDRSZ + strlen(buffer2) + 1);

   SET_VARSIZE(destination, VARHDRSZ + strlen(buffer2) + 1);
   mymemcpy(destination->vl_dat, buffer2, strlen(buffer2));
   // 添加标识位
   if (leftOpIsFloat || rightOpIsFloat)
   {
      destination->vl_dat[strlen(buffer2)] = '2';
   }
   else {
      destination->vl_dat[strlen(buffer2)] = '2';      
   }

   free(buffer);
   if (buffer2 != buffer)
   {
      free(buffer2);
   }

   PG_RETURN_TEXT_P(destination);
}

PG_FUNCTION_INFO_V1(smhe_pow);

Datum
    smhe_pow(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   int32 m1 = PG_GETARG_INT32(2);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;

   char *dat1 = palloc(s1 + 1);
   char *dat2 = palloc(s2 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   dat1[s1] = '\0';
   dat2[s2] = '\0';

   char *buffer = smhePow(dat1, dat2, m1); /* our source data */
   size_t buffSize = strlen(buffer);
   text *destination = (text *)palloc(VARHDRSZ + buffSize + 1);
   SET_VARSIZE(destination, VARHDRSZ + buffSize + 1);
   mymemcpy(destination->vl_dat, buffer, buffSize);

   free(buffer);
   PG_RETURN_TEXT_P(destination);
}

PG_FUNCTION_INFO_V1(smhe_inverse);

Datum
    smhe_inverse(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   // size_t s2 = VARSIZE(arg2) - VARHDRSZ;

   char *dat1 = palloc(s1 + 1);
   // char *dat2 = palloc(s2 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   // memcpy(dat2, arg2->vl_dat, s2);
   dat1[s1 - 1] = '\0';
   // dat2[s2] = '\0';

   char *buffer = smheInv(dat1); /* our source data */
   size_t buffSize = strlen(buffer);
   text *destination = (text *)palloc(VARHDRSZ + buffSize + 1);
   SET_VARSIZE(destination, VARHDRSZ + buffSize + 1);
   mymemcpy(destination->vl_dat, buffer, buffSize);

   free(buffer);
   free(dat1);
// 没有解决加法下放缩的问题，只能先设置为1了
destination->vl_dat[buffSize ] = '1';
   PG_RETURN_TEXT_P(destination);
}

PG_FUNCTION_INFO_V1(ore_comp);

Datum ore_comp(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   text *arg3 = PG_GETARG_TEXT_P(2);
   text *arg4 = PG_GETARG_TEXT_P(3);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;
   size_t s3 = VARSIZE(arg3) - VARHDRSZ;
   size_t s4 = VARSIZE(arg4) - VARHDRSZ;

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   memcpy(dat3, arg3->vl_dat, s3);
   memcpy(dat4, arg4->vl_dat, s4);

   int32 res = oreCompare(dat1, dat2, dat3, dat4, -1); /* our source data */

   PG_RETURN_INT32(res);
}

PG_FUNCTION_INFO_V1(udf_ore_lt);

Datum udf_ore_lt(PG_FUNCTION_ARGS)
{
   void* timer = startTimer();
   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   text *arg3 = PG_GETARG_TEXT_P(2);
   text *arg4 = PG_GETARG_TEXT_P(3);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;
   size_t s3 = VARSIZE(arg3) - VARHDRSZ;
   size_t s4 = VARSIZE(arg4) - VARHDRSZ;

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   memcpy(dat3, arg3->vl_dat, s3);
   memcpy(dat4, arg4->vl_dat, s4);

   int32 res = oreCompare(dat1, dat2, dat3, dat4, 1); /* our source data */

   bool judge = (res == -1) ? true : false;
c_count("interface_c: ore_lt", getPassedTime(timer, true));
   PG_RETURN_BOOL(judge);
}

PG_FUNCTION_INFO_V1(udf_ore_le);

Datum udf_ore_le(PG_FUNCTION_ARGS)
{
   void* timer = startTimer();
   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   text *arg3 = PG_GETARG_TEXT_P(2);
   text *arg4 = PG_GETARG_TEXT_P(3);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;
   size_t s3 = VARSIZE(arg3) - VARHDRSZ;
   size_t s4 = VARSIZE(arg4) - VARHDRSZ;

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   memcpy(dat3, arg3->vl_dat, s3);
   memcpy(dat4, arg4->vl_dat, s4);

   int32 res = oreCompare(dat1, dat2, dat3, dat4, 2); /* our source data */

   bool judge = (res == 1) ? false : true;
c_count("interface_c: ore_le", getPassedTime(timer, true));
   PG_RETURN_BOOL(judge);
}

PG_FUNCTION_INFO_V1(udf_ore_gt);

Datum udf_ore_gt(PG_FUNCTION_ARGS)
{
   void* timer = startTimer();

   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   text *arg3 = PG_GETARG_TEXT_P(2);
   text *arg4 = PG_GETARG_TEXT_P(3);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;
   size_t s3 = VARSIZE(arg3) - VARHDRSZ;
   size_t s4 = VARSIZE(arg4) - VARHDRSZ;

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   memcpy(dat3, arg3->vl_dat, s3);
   memcpy(dat4, arg4->vl_dat, s4);

   int32 res = oreCompare(dat1, dat2, dat3, dat4, 3); /* our source data */

   bool judge = (res == 1) ? true : false;
   c_count("interface_c: ore_gt", getPassedTime(timer, true));
   PG_RETURN_BOOL(judge);
}

PG_FUNCTION_INFO_V1(udf_ore_ge);

Datum udf_ore_ge(PG_FUNCTION_ARGS)
{
   void* timer = startTimer();

   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   text *arg3 = PG_GETARG_TEXT_P(2);
   text *arg4 = PG_GETARG_TEXT_P(3);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;
   size_t s3 = VARSIZE(arg3) - VARHDRSZ;
   size_t s4 = VARSIZE(arg4) - VARHDRSZ;

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   memcpy(dat3, arg3->vl_dat, s3);
   memcpy(dat4, arg4->vl_dat, s4);

   int32 res = oreCompare(dat1, dat2, dat3, dat4, 4); /* our source data */

   bool judge = (res == -1) ? false : true;

   c_count("interface_c: ore_ge", getPassedTime(timer, true));

   PG_RETURN_BOOL(judge);
}

PG_FUNCTION_INFO_V1(udf_ore_ee);

Datum udf_ore_ee(PG_FUNCTION_ARGS)
{
   text *arg1 = PG_GETARG_TEXT_P(0);
   text *arg2 = PG_GETARG_TEXT_P(1);
   text *arg3 = PG_GETARG_TEXT_P(2);
   text *arg4 = PG_GETARG_TEXT_P(3);

   size_t s1 = VARSIZE(arg1) - VARHDRSZ;
   size_t s2 = VARSIZE(arg2) - VARHDRSZ;
   size_t s3 = VARSIZE(arg3) - VARHDRSZ;
   size_t s4 = VARSIZE(arg4) - VARHDRSZ;

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);
   memcpy(dat1, arg1->vl_dat, s1);
   memcpy(dat2, arg2->vl_dat, s2);
   memcpy(dat3, arg3->vl_dat, s3);
   memcpy(dat4, arg4->vl_dat, s4);

   int32 res = oreCompare(dat1, dat2, dat3, dat4, 5); /* our source data */

   bool judge = (res == 0) ? true : false;

   PG_RETURN_BOOL(judge);
}
PG_FUNCTION_INFO_V1(ore_max_sfunc);
Datum ore_max_sfunc(PG_FUNCTION_ARGS) // sfunc
{
   text *interval = PG_GETARG_TEXT_P(0);
   text *interval_ore = NULL;
   text *interval_aes = NULL;
   split_to_2_text(interval, &interval_ore, &interval_aes);

   text *newVal_ore = PG_GETARG_TEXT_P(1);
   text *newVal_aes = PG_GETARG_TEXT_P(2);

   size_t s1 = VARSIZE(interval_ore) - VARHDRSZ;
   size_t s2 = VARSIZE(interval_aes) - VARHDRSZ;
   size_t s3 = VARSIZE(newVal_ore) - VARHDRSZ;
   size_t s4 = VARSIZE(newVal_aes) - VARHDRSZ;
   if (s2 == 0)
   {
      interval_ore = (text *)palloc(VARHDRSZ + s3);
      SET_VARSIZE(interval_ore, VARHDRSZ + s3);

      interval_aes = (text *)palloc(VARHDRSZ + s4);
      SET_VARSIZE(interval_aes, VARHDRSZ + s4);

      memcpy(interval_ore->vl_dat, newVal_ore->vl_dat, s3);
      memcpy(interval_aes->vl_dat, newVal_aes->vl_dat, s4);

      interval = concat_text(interval_ore, interval_aes);
      PG_RETURN_TEXT_P(interval);
   }

   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);

   memcpy(dat1, interval_ore->vl_dat, s1);
   memcpy(dat2, interval_aes->vl_dat, s2);
   memcpy(dat3, newVal_ore->vl_dat, s3);
   memcpy(dat4, newVal_aes->vl_dat, s4);

   assert(s2 == s4 && s1 == s3);
   int32 res = oreCompare(dat1, dat3, dat2, dat4, 6);
   if (res == -1)
   {
      interval_ore = (text *)palloc(VARHDRSZ + s3);
      SET_VARSIZE(interval_ore, VARHDRSZ + s3);

      interval_aes = (text *)palloc(VARHDRSZ + s4);
      SET_VARSIZE(interval_aes, VARHDRSZ + s4);

      memcpy(interval_ore->vl_dat, newVal_ore->vl_dat, s3);
      memcpy(interval_aes->vl_dat, newVal_aes->vl_dat, s4);
      interval = concat_text(interval_ore, interval_aes);
   }

   PG_RETURN_TEXT_P(interval);
}

PG_FUNCTION_INFO_V1(ore_max_final);
Datum ore_max_final(PG_FUNCTION_ARGS)
{
   text *data = PG_GETARG_TEXT_P(0);
   text *interval_ore = NULL;
   text *interval_aes = NULL;
   split_to_2_text(data, &interval_ore, &interval_aes);
   PG_RETURN_TEXT_P(interval_aes);
}

PG_FUNCTION_INFO_V1(ore_min_sfunc);

Datum ore_min_sfunc(PG_FUNCTION_ARGS) // sfunc
{
   text *interval = PG_GETARG_TEXT_P(0);
   text *interval_ore = NULL;
   text *interval_aes = NULL;
   split_to_2_text(interval, &interval_ore, &interval_aes);

   text *newVal_ore = PG_GETARG_TEXT_P(1);
   text *newVal_aes = PG_GETARG_TEXT_P(2);

   size_t s1 = VARSIZE(interval_ore) - VARHDRSZ;
   size_t s2 = VARSIZE(interval_aes) - VARHDRSZ;
   size_t s3 = VARSIZE(newVal_ore) - VARHDRSZ;
   size_t s4 = VARSIZE(newVal_aes) - VARHDRSZ;

   if (s2 == 0)
   {
      interval_ore = (text *)palloc(VARHDRSZ + s3);
      SET_VARSIZE(interval_ore, VARHDRSZ + s3);

      interval_aes = (text *)palloc(VARHDRSZ + s4);
      SET_VARSIZE(interval_aes, VARHDRSZ + s4);

      memcpy(interval_ore->vl_dat, newVal_ore->vl_dat, s3);
      memcpy(interval_aes->vl_dat, newVal_aes->vl_dat, s4);

      interval = concat_text(interval_ore, interval_aes);
      PG_RETURN_TEXT_P(interval);
   }
   assert(s2 == s4 && s1 == s3);
   char *dat1 = palloc0(s1 + 1);
   char *dat2 = palloc0(s2 + 1);
   char *dat3 = palloc0(s3 + 1);
   char *dat4 = palloc0(s4 + 1);

   memcpy(dat1, interval_ore->vl_dat, s1);
   memcpy(dat2, interval_aes->vl_dat, s2);
   memcpy(dat3, newVal_ore->vl_dat, s3);
   memcpy(dat4, newVal_aes->vl_dat, s4);

   int32 res = oreCompare(dat1, dat3, dat2, dat4, 7);
   if (res == 1)
   {
      interval_ore = (text *)palloc(VARHDRSZ + s3);
      SET_VARSIZE(interval_ore, VARHDRSZ + s3);

      interval_aes = (text *)palloc(VARHDRSZ + s4);
      SET_VARSIZE(interval_aes, VARHDRSZ + s4);

      memcpy(interval_ore->vl_dat, newVal_ore->vl_dat, s3);
      memcpy(interval_aes->vl_dat, newVal_aes->vl_dat, s4);
      interval = concat_text(interval_ore, interval_aes);
   }

   PG_RETURN_TEXT_P(interval);
}

PG_FUNCTION_INFO_V1(ore_min_final);
Datum ore_min_final(PG_FUNCTION_ARGS)
{
   text *data = PG_GETARG_TEXT_P(0);
   text *interval_ore = NULL;
   text *interval_aes = NULL;
   split_to_2_text(data, &interval_ore, &interval_aes);
   PG_RETURN_TEXT_P(interval_aes);
}

PG_FUNCTION_INFO_V1(sum_sahe_sfunc);

Datum sum_sahe_sfunc(PG_FUNCTION_ARGS)
{
   text *sum = PG_GETARG_TEXT_P(0);
   text *plus_val = PG_GETARG_TEXT_P(1);

   size_t s1 = VARSIZE(sum);
   size_t s2 = VARSIZE(plus_val);

   //printf("C:::::%s\n%s", sum->vl_dat, plus_val->vl_dat);
   // 当检测到null时自动将下一个值传给sum, 省去调用函数
   if (s1 < 10)
   {
      sum = (text *)palloc(VARHDRSZ + s2);
      SET_VARSIZE(sum, VARHDRSZ + s2);
      mymemcpy(sum->vl_dat, plus_val->vl_dat, s2);

      PG_RETURN_TEXT_P(sum);
   }

   char *dat1 = palloc(s1 + 1);
   char *dat2 = palloc(s2 + 1);
   memcpy(dat1, sum->vl_dat, s1);
   memcpy(dat2, plus_val->vl_dat, s2);
   dat1[s1] = '\0';
   dat2[s2] = '\0';

   char *interval_sum = SAHE_add_udf(dat1, dat2, s1 + 1, s2 + 1); //TODO: sum 未采用SGX

   sum = (text *)palloc(VARHDRSZ + s1 + 1);
   SET_VARSIZE(sum, VARHDRSZ + s1);

   // #ifdef ENCALVE
   // #else
   // #endif
   strcpy(sum->vl_dat, interval_sum);

   free(interval_sum);

   PG_RETURN_TEXT_P(sum);
}