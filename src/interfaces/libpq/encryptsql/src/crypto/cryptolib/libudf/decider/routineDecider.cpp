

#include "Timestamp.h"
#include "routineDecider.h"
#include "statusReport.h"
#include "App.h"
#include "Singleton.h"
#include "LogBase.h"
#include "config.h"
#include "EPCMonitor.h"
#include "TimeCounter.h"
#ifdef USE_SGX
#include "Enclave_u.h"
#endif
#include <unistd.h>
#include <vector>
#include <memory>

constexpr double RATIO = 0.8;

using namespace std;

string getUDFName(UdfKind u)
{
    if (u == SAHE_ADD)
        return "同态加";
    if (u == SAHE_MUL)
        return "SAHE_MUL";
    if (u == SAHE_SUB)
        return "SAHE_SUB";
    if (u == SAHE_NEG)
        return "SAHE_NEG";

    if (u == SMHE_MUL)
        return "同态乘";
    if (u == SMHE_MULP)
        return "SMHE_MULP";
    if (u == SMHE_DIV)
        return "SMHE_DIV";
    if (u == SMHE_POW)
        return "POW";
    if (u == SMHE_INV)
        return "SMHE_INV";
    if (u == ORE)
        return "ORE";
}
bool isFirstReport = true;
void RoutineDecider::report(std::unique_ptr<StatusReport> &rep)
{
    //排除Enclave初始化的影响
    if(isFirstReport) {
        isFirstReport = false;
        return;
    }
    
    auto t = Timer();
    auto udf = rep->_udfKind;
    auto &hwTime = op2Data[udf].avgHardWareTime;
    auto &swTime = op2Data[udf].avgSoftWareTime;

    // if (rep->_runningTime > hwTime * 10 || rep->_runningTime > swTime * 10)
    // {
    //     return;
    // }

    if (rep->_path == HARDWARE)
    {
        // std::unique_lock<std::shared_mutex> lock(op2Data[udf]._hMutex);
        ++op2Data[udf].htotal;
        // hwTime = RATIO * hwTime + (1 - RATIO) * rep->_runningTime;
        op2Data[udf].totalHardTime += rep->_runningTime / (double)Timestamp::kMicroSecondsPerSecond;
        if(rep->_runningTime ) {

        }
    }
    else
    {
        // std::unique_lock<std::shared_mutex> lock(op2Data[udf]._sMutex);
        ++op2Data[udf].stotal;
        // swTime = RATIO * swTime + (1 - RATIO) * rep->_runningTime;
        op2Data[udf].totalSoftTime += rep->_runningTime / (double)Timestamp::kMicroSecondsPerSecond;
        // op2Data[udf].softWareRawData.push_back(move(rep));
    }
    // std::unique_lock<std::mutex> lock(op2Data[udf]._logMutex);
    // op2Data[udf].runningLog.emplace_front(UDFExecutionLog{rep->_path, rep->_runningTime, swTime, hwTime});
    Singleton<TimeCounter>::instance()->count("decisionMake", t.passedTimeMicroSecond());
}

ExecutionPath RoutineDecider::makeDecision(StatusReport *rep)
{
    Timer t = Timer();
    // int pid =  getpid();
    // LogInfo("Pid:%d try to init EPCMonitor", pid);
    ExecutionPath path = SOFTWARE;
    auto udf = rep->_udfKind;
    double &avgSoftWareTime = op2Data[udf].avgSoftWareTime;
    double &avgHardWareTime = op2Data[udf].avgHardWareTime;
    {
        // std::shared_lock<std::shared_mutex>(op2Data[udf]._sMutex);
    }
#ifndef USE_SGX
    {
        // std::shared_lock<std::shared_mutex>(op2Data[udf]._hMutex);
        // double& avgHardWareTime = op2Data[udf].avgHardWareTime;
        // avgHardWareTime = INTMAX_MAX;
        path = SOFTWARE;
    }


#elif !defined(SGX_ORE)  // USE_SGX !SGX_ORE
// #elif 1

    EPCMonitor* pEPCMonitor = EPCMonitor::getInstance();

    path = avgSoftWareTime > avgHardWareTime ? HARDWARE : SOFTWARE;
    if( path == HARDWARE && pEPCMonitor->isEPCPaging()) {

        path = SOFTWARE;
    }
#else
    path = HARDWARE;
#endif
    string userSpecified = Config::getInstance()->Get("user.Execution","mix").asString();

    if(userSpecified == "sw") {
        path = SOFTWARE;
    }
    if(userSpecified == "hw") {
        path = HARDWARE;
    }
    Singleton<TimeCounter>::instance()->count("Report", t.passedTimeMicroSecond());
    return path;
}

RoutineDecider::~RoutineDecider()
{

#ifdef TIME_PORTRAIT
    // unordered_map<int, vector<double>> softWareRawData;
    int pid = getpid();
    char buf[256];
    sprintf(buf,"/etc/encryptsql/report-%d.log",pid);
    FILE *fp = fopen(buf, "w");
    sprintf(buf,"/etc/encryptsql/running-%d.log",pid);
    FILE *logFp = fopen(buf, "w");
    // Serializer serializer;
    long kms = Timestamp::kMicroSecondsPerSecond;
    fprintf(fp, "Prof That this is OK\n");
    if (!fp || !logFp)
    {
        util::Log("can not open report.log or running.log", util::log::Severity::error);
        abort();
    }

    for (auto &kv : op2Data)
    {
        string UdfName = getUDFName(kv.first);
        fprintf(fp, "UDF:%s\n", UdfName.c_str());
        double realAvgTimeH = kv.second.htotal == 0 ? 0 : (kv.second.totalHardTime * kms) / kv.second.htotal;
        double realAvgTimeS = kv.second.stotal == 0 ? 0 : (kv.second.totalSoftTime * kms) / kv.second.stotal;
        fprintf(fp, "硬件平均运行时间=%.6lfus, 累计平均时间=%.6lfus \n", realAvgTimeH, kv.second.avgHardWareTime);
        fprintf(fp, "软件平均运行时间=%.6lfus, 累计平均时间=%.6lfus \n", realAvgTimeS, kv.second.avgSoftWareTime);
        fprintf(fp, "硬件总计运行时间=%.3lfs\n", kv.second.totalHardTime);
        fprintf(fp, "软件总计运行时间=%.3lfs\n", kv.second.totalSoftTime);
        fprintf(fp, "总计执行次数：SOFT=%d，HARD=%d\n", kv.second.stotal, kv.second.htotal);
        fprintf(fp, "\n");

        // serializer.Set(UdfName + ".HWAvgTime", realAvgTimeH);
        // serializer.Set(UdfName + ".SWAvgTime", realAvgTimeS);
        // serializer.Set(UdfName + ".HWTotalTime", kv.second.totalHardTime);
        // serializer.Set(UdfName + ".SWTotalTime", kv.second.totalSoftTime);

        // serializer.Set(UdfName + ".HWTotal", kv.second.htotal);
        // serializer.Set(UdfName + ".SWTotal", kv.second.stotal);

        // serializer.ToFile("/etc/encryptsql/report.json");

        // long total = kv.second.stotal + kv.second.htotal;

        // auto &runLog = kv.second.runningLog;
        // fwrite((void *)&kv.first, 1, sizeof(int), logFp);
        // fwrite((void *)&total, 1, sizeof(long), logFp);
        // runLog.reverse();
        // size_t cnt = 0;
        // for (auto &it : runLog)
        // {
        //     ++cnt;
        //     fwrite((void *)&(it.path), 1, 4, logFp);
        //     fwrite((void *)&(it.runningTime), 3, sizeof(double), logFp);
        // }
    }
#ifdef USE_LRU
    double hit = -1;
    double swapout = -1;
    EnclaveManager *enclave = EnclaveManager::getInstance();
#ifdef USE_SGX
    ecall_getLruHit(enclave->getID(), &hit);
    ecall_getCacheSwapOut(enclave->getID(), &swapout);
#endif

    fprintf(fp, "Enclave Cache 命中率: %.2lf%%\n", hit * 100);
    fprintf(fp, "Enclave Cache 置换率: %.2lf%%\n", swapout * 100);
#endif
    fclose(fp);
    fclose(logFp);
#endif
}
