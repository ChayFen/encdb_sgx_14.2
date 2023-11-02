
#ifdef USE_SGX
#include "EPCMonitor.h"
#include "LogBase.h"
#include "shm.h"
#include "FileMutex.h"
#include <pthread.h>
#include <unistd.h>
using namespace util;
#include <cstdlib>

void genData(int64_t* arr, size_t size) {
    srand((int)time(NULL));
    for (int i = 0; i < size; i++)
    {
        arr[i] =  rand();
    }
}

void* test(void* ptr) {
    LogInfo("Pid:%d enter!", getpid());
}
static void sig_handler(int signo) {
    if(signo == SIGBUS) {
        LogWarning("SIGBUS received, ignore it");
    }
}
EPCMonitor::EPCMonitor() 
    : m_sleepTimeS(Config::getInstance()->Get("user.Monitor.sleepTimeS" ,1).asUInt64()),
        m_diffTimes(Config::getInstance()->Get("user.Monitor.diffTimes" ,1.5).asFloat())
{
    // if (signal(SIGBUS, sig_handler) == SIG_ERR ) {
    //     LogWarning("can not catch SIGBUS");
    // }
    int pid = gettid();
    m_isRunning.store(true);
    FileMutexLocker fileMutex;
    m_isPaging = &m_shm.getHandle()->EPCPaging;
    if(access(UNIQUE_LOCK, F_OK ) == -1 && fileMutex.lock()) {
        LogInfo("Pid:%d init the mutex and epcMonitor", pid);
        m_pShmMutex = new ShmMutex(m_shm.getHandle());
        LogInfo("Pid:%d inited the mutex ", pid);
        pthread_create(&m_worker, NULL, EPCMonitor::threadHelper, this);
        // test
        LogInfo("Pid:%d init done", pid);
        m_shm.getHandle()->epcThread = pid;
        auto fp = fopen(UNIQUE_LOCK, "w");
        fprintf(fp, "%d", pid);
        fclose(fp);
        fileMutex.unlock();
    }
    else {
         LogInfo("Pid:%d didnt get Locker", pid);
        
        while(access(UNIQUE_LOCK, F_OK ) == -1) {
            LogInfo("Pid:%d uniquelock is not exist sleep", pid);
            sleep(3);
        }
        LogInfo("Pid:%d sleeped", pid);
        m_pShmMutex = new ShmMutex();
        m_pShmMutex->setHandle(m_shm.getHandle());
    }
}

void EPCMonitor::run() {
    LogInfo("Pid:%d EPCMonitor is starting...", getpid());
    while (m_isRunning.load())
    {
        Timer t;
        m_task();
        auto runningTime = t.passedTimeMicroSecond();
        auto& normal = m_task.m_normalRunningTime;
        auto& paging = m_task.m_pagingRunningTime;
        

        if(normal == 0) {
            normal = runningTime;
            sleep(m_sleepTimeS);
            continue;
        }
        if(runningTime <= normal * m_diffTimes) { 
            normal = normal * 0.5 + runningTime * 0.5;
            unsetEPCPaging();
            LogInfo("UnSet Pageing, cur:%lf, normal:%lf, page:%lf", runningTime, normal, paging );
            m_normalCnt++;
        }
        else {  // paging
            if(paging == 0) {
                paging = runningTime;
            }
            else 
                paging = paging * 0.5 + runningTime * 0.5;
            setEPCPaging();
            LogInfo("Set Pageing, cur:%lf, normal:%lf, page:%lf", runningTime, normal, paging );
            m_pagingCnt++;
        }
        m_record.push_back(runningTime); //TODO: debug
        m_record.push_back(normal); //TODO: debug
        m_record.push_back(paging); //TODO: debug
        sleep(m_sleepTimeS);
    }
}

Task::Task() 
{
    auto pConfig = Config::getInstance();
    m_normalRunningTime = pConfig->Get("user.Monitor.normalRunningTime",0).asFloat();
    m_pagingRunningTime = pConfig->Get("user.Monitor.pagingRunningTime",0).asFloat();
    m_taskDataSize = pConfig->Get("user.Monitor.taskDataSize",1024).asUInt64();
    m_data.resize(m_taskDataSize);
    genData(m_data.data(), m_taskDataSize);
}

void Task::operator()() {
    auto enclave =  EnclaveManager::getInstance();
    auto copied = m_data;
    enclave->trusted_qsort(copied.data(), copied.size());
}

#endif