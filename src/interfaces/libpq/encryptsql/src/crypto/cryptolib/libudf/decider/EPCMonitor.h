

#ifdef USE_SGX
#pragma once
#include "EnclaveManager.h"
#include "Timer.h"
#include "config.h"
#include <functional>
#include <atomic>
#include <time.h>
#include <thread>
#include "shm.h"

#define UNIQUE_LOCK "/tmp/encryptsql/init.lock"

using namespace std;
using namespace util;
struct Task {

    enum TaskType {
        BSEARCH,
        QSORT,
        MIX
    };
    double m_normalRunningTime = 0;
    double m_pagingRunningTime = 0;
    size_t m_taskDataSize = 1024;
    TaskType m_taskKinds = QSORT;
    vector<int64_t> m_data;
    // std::function<void(void)> doTask;
    Task();
    void operator()();
};

class EPCMonitor {
// typedef std::function<void(void)> Task_t;

public:
    static EPCMonitor* getInstance() {
        static EPCMonitor monitor;
        return &monitor;
    }

    static void* threadHelper(void* ptr) {
        LogInfo("Pid:%d threadHelper", getpid());
        ((EPCMonitor*)ptr)->run();
        return NULL;
    }

    void setEPCPaging() {
        m_pShmMutex->lock();
        // m_isPaging.store(true);
        *m_isPaging = true;
        m_pShmMutex->unlock();
    }
    void unsetEPCPaging() {
        m_pShmMutex->lock();
        *m_isPaging = false;
        m_pShmMutex->unlock();
        // m_isPaging.store(false);
    }
    void stop() {
        m_isRunning.store(false);
    }
    bool isEPCPaging() {
        // bool ret;
        // m_pShmMutex->lock();
        // // m_isPaging.store(true);
        bool ret = *m_isPaging;
        // m_pShmMutex->unlock();
        return ret;
        // return m_isPaging.load();
    }

private:
    EPCMonitor() ;
    EPCMonitor(const EPCMonitor &r);
    ~EPCMonitor(){
        stop();
        remove(UNIQUE_LOCK);
        int pid = getpid();
        char buf[256];
        sprintf(buf,"/etc/encryptsql/epcMonitor-%d.log",pid);
        FILE *fp = fopen(buf, "w");
        size_t size = m_record.size();
        fwrite((void *)&size, 1, sizeof(long), fp);
        fwrite((void *)m_record.data(), size, sizeof(double), fp);
        fclose(fp);
    }
    void run();
    
private:
    bool* m_isPaging = nullptr;
    atomic<bool> m_isRunning = false;
    vector<double> m_record; //TODO: for debug
    uint64_t m_sleepTimeS = 1;
    double m_diffTimes;
    size_t m_normalCnt = 0;
    size_t m_pagingCnt = 0;
    Task m_task;
    // thread m_worker;
    pthread_t m_worker;
    SharedMemory m_shm;
    ShmMutex* m_pShmMutex;
};
#endif