//
// Created by 甜甜 on 2020/12/13.
//

#ifndef SYMMETRIA_2_STR22STRUCT_H
#define SYMMETRIA_2_STR22STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <memory>

#include <map>
#include <vector>
#include <string.h>
#include <string>
#include <unordered_map>

//#include "split.h"
using namespace std;

// memset(*dest, 0, size);

inline void memset(void *dest, char c, size_t size)
{
    for (int i = 0; i < size; i++)
        ((char *)dest)[i] = c;
}



class str22struct
{
public:
    struct ArraySymCipher_change
    {
        long long value;
        int sizePos;
        long long offsetPos;
        vector<int> idsPos;
        //int idsPos[256];
        long long cardMultiplierPos;
        unordered_map<int, long long> cardPos;
        int sizeNeg;
        long long offsetNeg;
        vector<int> idsNeg;
        //int idsNeg[256];
        long long cardMultiplierNeg;
        unordered_map<int, long long> cardNeg;
    };
    string getString(const int &a);
    string getString(const long long &a);
    string getString(const string &s);

    template <typename TypeOne, typename TypeTwo>
    //    string mapToString(unordered_map<TypeOne,TypeTwo>& m);
    string mapToString(unordered_map<TypeOne, TypeTwo> &m)
    {
        string str = "";
        typename unordered_map<TypeOne, TypeTwo>::iterator it = m.begin();
        for (; it != m.end(); it++)
        {
            // str += "<";
            str += getString(it->first) + "_" + getString(it->second) + "?";
            //str += ">";
        }
        if (m.empty())
        {
            str = "LL";
        }
        return str;
    }

    string ArraySymCipher_to_str(char **dest, struct ArraySymCipher_change *cipher);
    char *strsep(char **stringp, const char *delim);
    void pushToVec(std::vector<std::string> &obj, const char *param, std::string token);
    vector<string> split(string s, char delim);
    int ArraySymCipher_parse(string str, struct ArraySymCipher_change **cipher);
};

#endif //SYMMETRIA_2_STR22STRUCT_H
