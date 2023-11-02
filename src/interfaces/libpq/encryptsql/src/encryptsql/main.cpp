#include "encryptsql.h"
#include <iostream>
#include <pthread.h>
#ifdef __cplusplus
extern "C"
{
#endif
#include "postgres.h" // 与 #include <iomanip> 冲突

#include "utils/palloc.h"
#include "utils/memutils.h"

#include "nodes/nodes.h"
#include "encryptsql/node2str.h"

#include "utils/memutils.h"

#ifdef __cplusplus
}
#endif
#include <unistd.h>

#include "utils.h"
#include "encryptsql/fieldmap.h"

#include "Singleton.h"
#include "LRUCache.hpp"
#include "KeyManager.h"
#include "SymAHE.h"
#include "TimeCounter.h"

extern void LogInfo(const char* fmt, ...);

constexpr int MEMORY_DELETE_LIMIT = 20000;

__thread size_t sql_processed_counter = 0;
#ifdef USE_LRU
// extern LRUCache<long, long> HECache;           // for RandNum(id)  
extern LRUCache<long, std::string> AESCache;   // for AES decrypt
#endif

#ifndef MAPPERPATH
#define MAPPERPATH "/etc/encryptsql/map.json"
#endif

extern "C" List *raw_parser(const char *str);

extern Node *encryptStmt(Node *parseTree, EncryptInfo* info);

extern const char *parseTree2str(Node *parseTree);

extern "C" const char *encryptOneSql(const char *sql) __attribute__((visibility("default")));

extern "C" void disableAsNeeded() __attribute__((visibility("default")));

__thread MemoryContext EncryptContext = NULL;

static __thread bool Init = false;
__thread bool Encrypt = true;
__thread bool SkipDecryptRes = false;

void disableAsNeeded()
{
    if (!Init)
        std::cout << "Encryption Library Linked\n";
    return;
}


bool needDecryptResult(Node *node)
{

    if (node->type == T_ExplainStmt)
    {
        return false;
    }
    return true;
}

bool needSkipEncryption(const char *sql, Node *node = nullptr)
{
    if (sql)
    {
        if (isFilteredSql(sql))
        {
            return true;
        }
    }
    if (node)
    {
        if (node->type == T_VariableShowStmt)
        {
            return true;
        }
    }

    return false;
}

const char *
encryptOneSql(const char *sql)
{
    //q:用于时间测量和计数，可能用于性能分析？
    TimeCounter *counter = Singleton<TimeCounter>::instance();
    Timer globalTimer;
    #ifndef USE_ENCRYPT
    Encrypt = false;
    return sql;
    #endif
    std::cout << "main start\n";
    SkipDecryptRes = false;
    //q:检查是否为特殊命令，是的话跳过加密
    if (checkCmd(sql, &Encrypt))
    {
        SkipDecryptRes = true;
        return Encrypt ? "select 1 as enc_status;" : "select 0 as enc_status;"; // 这是设置transform的语句
    }

    if (!Encrypt)
        return sql;

    //q:初始化语法树、加密语法树和结果字符串
    Node *parseTree = NULL;
    Node *encParseTree = NULL;
    const char *res = NULL;
    
    //q:获取进程id，仅debug模式会调用
    long long tid = gettid();
    __pid_t pid = getpid();
    // debug2file(sql, true);
    // LogInfo("TID:%d, PID:%d, SQL RECV:%s", tid, pid, sql);

    //q:内存上下文的初始化，映射器的初始化，不太懂
    if (!Init)
    {
        MemoryContextInit();
        MessageContext = AllocSetContextCreate(TopMemoryContext,
                                               "MessageContext",
                                               ALLOCSET_DEFAULT_MINSIZE,
                                               ALLOCSET_DEFAULT_INITSIZE,
                                               ALLOCSET_DEFAULT_MAXSIZE);
        EncryptContext = AllocSetContextCreate(TopMemoryContext,
                                               "EncryptContext",
                                               ALLOCSET_DEFAULT_MINSIZE,
                                               ALLOCSET_DEFAULT_INITSIZE,
                                               ALLOCSET_DEFAULT_MAXSIZE);

        //q:映射器初始化，后文明文对应密文应该是这里
        mapperInit(MAPPERPATH);

#ifdef USE_LRU

    // HECache.setName("Client HE");           // for RandNum(id)  
    AESCache.setName("Client AES"); 
#endif
        Init = true;
    }
    sql_processed_counter++;

    //q:根据全局变量判断内存是否需要重置或者删除
    if (sql_processed_counter < MEMORY_DELETE_LIMIT)
    {
        MemoryContextResetChildren(TopMemoryContext);
        // MemoryContextReset(MessageContext);
    }
    else
    {
        sql_processed_counter = 0;
        MemoryContextDeleteChildren(TopMemoryContext);
        // MemoryContextDelete(MessageContext);
    }

    if (needSkipEncryption(sql))
    {
        SkipDecryptRes = true;
        return sql;
    }

    //q:切换内存上下文到上文初始化的MessageContext
    MemoryContextSwitchTo(MessageContext);

    Timer t;
    //q:调用raw_paser返回语法树
    List *AST = (List *)raw_parser(sql);

    if (AST)
        //q:获取AST的首元素:SELECT,UPDATE,DELETE.......
        parseTree = (Node *)linitial(AST);
    else
    {
        SkipDecryptRes = true;
        errmsg("error when parse SQL:\n %s\n", sql);
    }
    if (parseTree)
    {
        //q:为什么要频繁切换内存上下文？
        MemoryContextSwitchTo(EncryptContext);
        Timer t;
        //q:判断指针是否合法，以及是否需要加密，具体内容待细看
        if (needSkipEncryption(nullptr, parseTree))
        {
            SkipDecryptRes = true;
            return sql;
        }
        EncryptInfo *info = (EncryptInfo *)palloc0(sizeof(EncryptInfo));
        info->sql = sql;
        encParseTree = encryptStmt(parseTree, info);
        counter->count("SQL Encryption", t.passedTimeMicroSecond());
        if (!needDecryptResult(parseTree))
        {
            SkipDecryptRes = true;
        }
        // mapperSave();
    }

    if (encParseTree)
    {
        Timer t;
        res = parseTree2str(encParseTree);
        counter->count("SQL AST To String", t.passedTimeMicroSecond());
    }

    if (!res)
        SkipDecryptRes = true;

    const char *retStuff = res ? res : sql;
#ifndef NDEBUG
    
    std::cout << "PID:" << tid << std::endl;
    debug2file(sql, true);
    std::cout << sql << std::endl;

    debug2file(retStuff, false);
    std::cout << retStuff << std::endl;
    std::cout << "main done\n";
#endif
    std::cout << retStuff << std::endl;
    return retStuff;
}
