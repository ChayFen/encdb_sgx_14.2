//
// Created by 甜甜 on 2020/12/14.
//
////UDF需要调用的c++相关接口函数
///加解密测试

#include <vector>
#include <string.h>
#include "MathUtils.h"
#include "Symcipher.h"
#include "ArraySymCipher.h"
#include "str22struct.h"
#include "SymPHE.h"
#include "SymAHE.h"
#include "SymMHE.h"

#include "t.h"

#include "App.h"

//#include <sstream>

#include <string.h>


using namespace std;

extern "C"
{

#include "Interface.h"
#ifdef USE_SGX
#include "Enclave_u.h"
#endif
}

char *SAHE_add_udf(char *cipher1, char *cipher2, size_t csz1, size_t csz2)
{
    char *str = NULL;
    string res;
    string structTostr;

    SymCipher *a = new ArraySymCipher(cipher1);
    SymCipher *b = new ArraySymCipher(cipher2);

    SymAHE symAHE = SymAHE();

    symAHE.add(*a, *b);
    str22struct::ArraySymCipher_change p;

    p.value = a->getValue();
    p.sizePos = a->getsizePos();
    p.offsetPos = a->getoffsetPos();
    p.cardMultiplierPos = a->getcardMultiplierPos();
    p.sizeNeg = a->getsizeNeg();
    p.offsetNeg = a->getoffsetNeg();
    p.cardMultiplierNeg = a->getcardMultiplierNeg();
    p.idsPos.assign(a->getidsPos().begin(), a->getidsPos().end());
    p.idsNeg.assign(a->getidsNeg().begin(), a->getidsNeg().end());
    p.cardPos = a->getcardPos();
    p.cardNeg = a->getcardNeg();
    str22struct str22;
    string mapTostrPos = str22.mapToString(p.cardPos);
    string mapTostrNeg = str22.mapToString(p.cardNeg);

    res = str22.ArraySymCipher_to_str(&str, &p);
    structTostr = res + mapTostrPos + "%" + mapTostrNeg + "^";
    //char *ch=(char*)structTostr.data();


    delete a;
    delete b;

    //! WBUG: 在UDF中采用new分配内存会不会有问题？
    char *buf = new char[csz1 * 2];
    sprintf(buf, "%s%s%%%s^", res.c_str(), mapTostrPos.c_str(), mapTostrNeg.c_str());
    return buf;
}

char *SAHE_addplaintext_udf(char *cipher1, int m,  size_t csz1)
{ // post
    //
    str22struct str22;
    
    //// " << cipher1 << endl;

    char *str = NULL;
    string res;
    string structTostr;

    SymCipher *a = new ArraySymCipher(cipher1);

    SymAHE symAHE = SymAHE();

    symAHE.addPlaintext(*a, m);
    str22struct::ArraySymCipher_change p;

    p.value = a->getValue();
    p.sizePos = a->getsizePos();
    p.offsetPos = a->getoffsetPos();
    p.cardMultiplierPos = a->getcardMultiplierPos();
    p.sizeNeg = a->getsizeNeg();
    p.offsetNeg = a->getoffsetNeg();
    p.cardMultiplierNeg = a->getcardMultiplierNeg();
    p.idsPos.assign(a->getidsPos().begin(), a->getidsPos().end());
    p.idsNeg.assign(a->getidsNeg().begin(), a->getidsNeg().end());
    p.cardPos = a->getcardPos();
    p.cardNeg = a->getcardNeg();

    string mapTostrPos = str22.mapToString(p.cardPos);
    string mapTostrNeg = str22.mapToString(p.cardNeg);

    res = str22.ArraySymCipher_to_str(&str, &p);
    structTostr = res + mapTostrPos + "%" + mapTostrNeg + "^";
    //char *ch=(char*)structTostr.data();

    delete a;
    
    char *buf = new char[csz1 * 2];
    sprintf(buf, "%s%s%%%s^", res.c_str(), mapTostrPos.c_str(), mapTostrNeg.c_str());
    return buf;
}

char *SAHE_substract_udf(char *cipher1, char *cipher2, size_t csz1, size_t csz2)
{ // post
    
    
    
    
    
    
    str22struct str22;
    
    
    

    char *str = NULL;
    string res;
    string structTostr;


    SymCipher *a = new ArraySymCipher(cipher1);
    SymCipher *b = new ArraySymCipher(cipher2);

    SymAHE symAHE = SymAHE();

    symAHE.subtract(*a, *b);
    str22struct::ArraySymCipher_change p;

    p.value = a->getValue();
    p.sizePos = a->getsizePos();
    p.offsetPos = a->getoffsetPos();
    p.cardMultiplierPos = a->getcardMultiplierPos();
    p.sizeNeg = a->getsizeNeg();
    p.offsetNeg = a->getoffsetNeg();
    p.cardMultiplierNeg = a->getcardMultiplierNeg();
    p.idsPos.assign(a->getidsPos().begin(), a->getidsPos().end());
    p.idsNeg.assign(a->getidsNeg().begin(), a->getidsNeg().end());
    p.cardPos = a->getcardPos();
    p.cardNeg = a->getcardNeg();

    string mapTostrPos = str22.mapToString(p.cardPos);
    string mapTostrNeg = str22.mapToString(p.cardNeg);

    res = str22.ArraySymCipher_to_str(&str, &p);
    structTostr = res + mapTostrPos + "%" + mapTostrNeg + "^";
    //char *ch=(char*)structTostr.data();
    delete a;
    delete b;
    
    
        char *buf = new char[csz1 * 2];
    sprintf(buf, "%s%s%%%s^", res.c_str(), mapTostrPos.c_str(), mapTostrNeg.c_str());
    return buf;
}
char *SAHE_multiply_udf(char *cipher1, int m,  size_t csz1)
{ // post
    
    
    
    
    str22struct str22;
    
    //// " << cipher1 << endl;

    char *str = NULL;
    string res;
    string structTostr;
   
    SymCipher *a = new ArraySymCipher(cipher1);

    SymAHE symAHE = SymAHE();

    symAHE.multiply(*a, m);
    str22struct::ArraySymCipher_change p;

    p.value = a->getValue();
    p.sizePos = a->getsizePos();
    p.offsetPos = a->getoffsetPos();
    p.cardMultiplierPos = a->getcardMultiplierPos();
    p.sizeNeg = a->getsizeNeg();
    p.offsetNeg = a->getoffsetNeg();
    p.cardMultiplierNeg = a->getcardMultiplierNeg();
    p.idsPos.assign(a->getidsPos().begin(), a->getidsPos().end());
    p.idsNeg.assign(a->getidsNeg().begin(), a->getidsNeg().end());
    p.cardPos = a->getcardPos();
    p.cardNeg = a->getcardNeg();

    string mapTostrPos = str22.mapToString(p.cardPos);
    string mapTostrNeg = str22.mapToString(p.cardNeg);

    res = str22.ArraySymCipher_to_str(&str, &p);
    structTostr = res + mapTostrPos + "%" + mapTostrNeg + "^";
    //char *ch=(char*)structTostr.data();

    
    
    
    

    delete a;
    
        char *buf = new char[csz1 * 2];
    sprintf(buf, "%s%s%%%s^", res.c_str(), mapTostrPos.c_str(), mapTostrNeg.c_str());
    return buf;
}
char *SAHE_neggate_udf(char *cipher1, size_t csz1)
{ 
    
    
    
    
    str22struct str22;
    
    //cout<<"cipher1_parse: "<<cipher1<<endl;

    char *str = NULL;
    string res;
    string structTostr;



    SymCipher *a = new ArraySymCipher(cipher1);

    SymAHE symAHE = SymAHE();
    //SymCipher &c_ahe3 = symAHE.encrypt(1001);

    symAHE.neggate(*a);
    str22struct::ArraySymCipher_change p;

    p.value = a->getValue();
    p.sizePos = a->getsizePos();
    p.offsetPos = a->getoffsetPos();
    p.cardMultiplierPos = a->getcardMultiplierPos();
    p.sizeNeg = a->getsizeNeg();
    p.offsetNeg = a->getoffsetNeg();
    p.cardMultiplierNeg = a->getcardMultiplierNeg();
    p.idsPos.assign(a->getidsPos().begin(), a->getidsPos().end());
    p.idsNeg.assign(a->getidsNeg().begin(), a->getidsNeg().end());
    p.cardPos = a->getcardPos();
    p.cardNeg = a->getcardNeg();

    string mapTostrPos = str22.mapToString(p.cardPos);
    string mapTostrNeg = str22.mapToString(p.cardNeg);

    res = str22.ArraySymCipher_to_str(&str, &p);
    structTostr = res + mapTostrPos + "%" + mapTostrNeg + "^";
    //char *ch=(char*)structTostr.data();

    delete a;
    
        char *buf = new char[csz1 * 2];
    sprintf(buf, "%s%s%%%s^", res.c_str(), mapTostrPos.c_str(), mapTostrNeg.c_str());
    return buf;
}
char *SMHE_MULTIPLY_udf(char *cipher1, char *cipher2, size_t csz1, size_t csz2)
{ // post

    str22struct str22;


    char *str = NULL;
    string res;
    string structTostr;

    SymCipher *a = new ArraySymCipher(cipher1);
    SymCipher *b = new ArraySymCipher(cipher2);

    SymMHE symMHE = SymMHE();

    symMHE.multiply(*a, *b);
    str22struct::ArraySymCipher_change p;

    p.value = a->getValue();
    p.sizePos = a->getsizePos();
    p.offsetPos = a->getoffsetPos();
    p.cardMultiplierPos = a->getcardMultiplierPos();
    p.sizeNeg = a->getsizeNeg();
    p.offsetNeg = a->getoffsetNeg();
    p.cardMultiplierNeg = a->getcardMultiplierNeg();
    p.idsPos.assign(a->getidsPos().begin(), a->getidsPos().end());
    p.idsNeg.assign(a->getidsNeg().begin(), a->getidsNeg().end());
    p.cardPos = a->getcardPos();
    p.cardNeg = a->getcardNeg();

    string mapTostrPos = str22.mapToString(p.cardPos);
    string mapTostrNeg = str22.mapToString(p.cardNeg);

    res = str22.ArraySymCipher_to_str(&str, &p);
    structTostr = res + mapTostrPos + "%" + mapTostrNeg + "^";
    //char *ch=(char*)structTostr.data();
    delete a;
    delete b;
    //! LEAK
        char *buf = new char[csz1 * 2];
    sprintf(buf, "%s%s%%%s^", res.c_str(), mapTostrPos.c_str(), mapTostrNeg.c_str());
    return buf;
}

char *SMHE_MULTIPLYPLAIN_udf(char *cipher1, int m,  size_t csz1)
{
    str22struct str22;

    char *str = NULL;
    string res;
    string structTostr;

    SymCipher *a = new ArraySymCipher(cipher1);

    SymMHE symMHE = SymMHE();

    symMHE.multiplyPlaintext(*a, m);
    str22struct::ArraySymCipher_change p;

    p.value = a->getValue();
    p.sizePos = a->getsizePos();
    p.offsetPos = a->getoffsetPos();
    p.cardMultiplierPos = a->getcardMultiplierPos();
    p.sizeNeg = a->getsizeNeg();
    p.offsetNeg = a->getoffsetNeg();
    p.cardMultiplierNeg = a->getcardMultiplierNeg();
    p.idsPos.assign(a->getidsPos().begin(), a->getidsPos().end());
    p.idsNeg.assign(a->getidsNeg().begin(), a->getidsNeg().end());
    p.cardPos = a->getcardPos();
    p.cardNeg = a->getcardNeg();

    string mapTostrPos = str22.mapToString(p.cardPos);
    string mapTostrNeg = str22.mapToString(p.cardNeg);

    res = str22.ArraySymCipher_to_str(&str, &p);
    structTostr = res + mapTostrPos + "%" + mapTostrNeg + "^";
    //char *ch=(char*)structTostr.data();
    delete a;
        char *buf = new char[csz1 * 2];
    sprintf(buf, "%s%s%%%s^", res.c_str(), mapTostrPos.c_str(), mapTostrNeg.c_str());
    return buf;
}

char *SMHE_DIVIDE_udf(char *cipher1, char *cipher2, size_t csz1, size_t csz2)
{
    
    
    

    str22struct str22;

    char *str = NULL;
    string res;
    string structTostr;

    SymCipher *a = new ArraySymCipher(cipher1);
    SymCipher *b = new ArraySymCipher(cipher2);

    SymMHE symMHE = SymMHE();

    symMHE.divide(*a, *b);
    str22struct::ArraySymCipher_change p;

    p.value = a->getValue();
    p.sizePos = a->getsizePos();
    p.offsetPos = a->getoffsetPos();
    p.cardMultiplierPos = a->getcardMultiplierPos();
    p.sizeNeg = a->getsizeNeg();
    p.offsetNeg = a->getoffsetNeg();
    p.cardMultiplierNeg = a->getcardMultiplierNeg();
    p.idsPos.assign(a->getidsPos().begin(), a->getidsPos().end());
    p.idsNeg.assign(a->getidsNeg().begin(), a->getidsNeg().end());
    p.cardPos = a->getcardPos();
    p.cardNeg = a->getcardNeg();

    string mapTostrPos = str22.mapToString(p.cardPos);
    string mapTostrNeg = str22.mapToString(p.cardNeg);

    res = str22.ArraySymCipher_to_str(&str, &p);
    structTostr = res + mapTostrPos + "%" + mapTostrNeg + "^";
    //char *ch=(char*)structTostr.data();

    delete a;
    delete b;
        char *buf = new char[csz1 * 2];
    sprintf(buf, "%s%s%%%s^", res.c_str(), mapTostrPos.c_str(), mapTostrNeg.c_str());
    return buf;
}

char *SMHE_POW_udf(char *cipher1, int m,  size_t csz1)
{
    str22struct str22;

    char *str = NULL;
    string res;
    string structTostr;

    SymCipher *a = new ArraySymCipher(cipher1);

    SymMHE symMHE = SymMHE();

    symMHE.pow(*a, m);
    str22struct::ArraySymCipher_change p;

    p.value = a->getValue();
    p.sizePos = a->getsizePos();
    p.offsetPos = a->getoffsetPos();
    p.cardMultiplierPos = a->getcardMultiplierPos();
    p.sizeNeg = a->getsizeNeg();
    p.offsetNeg = a->getoffsetNeg();
    p.cardMultiplierNeg = a->getcardMultiplierNeg();
    p.idsPos.assign(a->getidsPos().begin(), a->getidsPos().end());
    p.idsNeg.assign(a->getidsNeg().begin(), a->getidsNeg().end());
    p.cardPos = a->getcardPos();
    p.cardNeg = a->getcardNeg();

    string mapTostrPos = str22.mapToString(p.cardPos);
    string mapTostrNeg = str22.mapToString(p.cardNeg);

    res = str22.ArraySymCipher_to_str(&str, &p);
    structTostr = res + mapTostrPos + "%" + mapTostrNeg + "^";
    //char *ch=(char*)structTostr.data();

    
    
    
    

    delete a;
    
        char *buf = new char[csz1 * 2];
    sprintf(buf, "%s%s%%%s^", res.c_str(), mapTostrPos.c_str(), mapTostrNeg.c_str());
    return buf;
}

char *SMHE_INVERSE_udf(char *cipher1, size_t csz1)
{ // post
    
    
    
    
    str22struct str22;
    
    //cout<<"cipher1_parse: "<<cipher1<<endl;

    char *str = NULL;
    string res;
    string structTostr;


    SymCipher *a = new ArraySymCipher(cipher1);

    SymMHE symMHE = SymMHE();
    symMHE.inverse(*a);
    str22struct::ArraySymCipher_change p;

    p.value = a->getValue();
    p.sizePos = a->getsizePos();
    p.offsetPos = a->getoffsetPos();
    p.cardMultiplierPos = a->getcardMultiplierPos();
    p.sizeNeg = a->getsizeNeg();
    p.offsetNeg = a->getoffsetNeg();
    p.cardMultiplierNeg = a->getcardMultiplierNeg();
    p.idsPos.assign(a->getidsPos().begin(), a->getidsPos().end());
    p.idsNeg.assign(a->getidsNeg().begin(), a->getidsNeg().end());
    p.cardPos = a->getcardPos();
    p.cardNeg = a->getcardNeg();

    string mapTostrPos = str22.mapToString(p.cardPos);
    string mapTostrNeg = str22.mapToString(p.cardNeg);

    res = str22.ArraySymCipher_to_str(&str, &p);
    structTostr = res + mapTostrPos + "%" + mapTostrNeg + "^";
    //char *ch=(char*)structTostr.data();

    
    
    
    

    delete a;
    
    char *buf = new char[csz1 * 2];
    sprintf(buf, "%s%s%%%s^", res.c_str(), mapTostrPos.c_str(), mapTostrNeg.c_str());
    return buf;
}
