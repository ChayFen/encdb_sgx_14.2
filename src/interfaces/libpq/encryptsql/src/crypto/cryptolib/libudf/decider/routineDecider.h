
#pragma once

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <memory>
#include <forward_list>
#include "statusReport.h"



class RoutineDecider {

    struct UDFExecutionLog {
        ExecutionPath path;
        double runningTime;
        double swAvgTime;
        double hwAvgTime;
    };

    struct UDFExecutionStatistics
    {
        // std::vector<std::unique_ptr<StatusReport>> hardWareRawData;    // 原始数据链表
        // std::vector<std::unique_ptr<StatusReport>> softWareRawData;    // 原始数据链表

        double avgHardWareTime = 0; // 每个操作的软硬实现平均时长
        double avgSoftWareTime = 0;
        double totalHardTime = 0;
        double totalSoftTime = 0;

        int stotal = 0; // 软件运行计数
        int htotal = 0; // 硬件运行计数

        std::shared_mutex _hMutex; // hardware lock
        std::shared_mutex _sMutex; // software lock
        std::mutex _logMutex; // for runningLog
        std::forward_list<UDFExecutionLog> runningLog; // 第i次运行的数据。ith -> <time, path>
    };

    std::unordered_map<UdfKind, UDFExecutionStatistics> op2Data;

public:
    void report(std::unique_ptr<StatusReport> &rep);
    RoutineDecider() {
        fillWithInitValue();
    }
    ~RoutineDecider();
    ExecutionPath makeDecision(StatusReport *rep);

private:
    void fillWithInitValue() {
        op2Data[ORE].avgHardWareTime = 8;
        op2Data[ORE].avgSoftWareTime = 12;

        op2Data[SAHE_ADD].avgHardWareTime = 19;
        op2Data[SAHE_ADD].avgSoftWareTime = 46;
    }
};

// #endif

// #ifdef __cpluscplus
// extern "C" {
// #endif
// void report(struct StatusReport *rep);
// int makeDecision(struct StatusReport *rep);

// #ifdef __cpluscplus
// }
// #endif
