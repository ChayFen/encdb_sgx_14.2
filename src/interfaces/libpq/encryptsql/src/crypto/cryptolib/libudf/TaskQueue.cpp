#ifdef ECALL_BATCH

#include "TaskQueue.h"

#include "LogBase.h"
#include <string>
#include <utility>
#include <mutex>
#include <thread>
#include <functional>

namespace taskpool {

constexpr int QUEUE_SIZE = 10000;

mutex queueMutex;
TaskQueue g_taskQueue(QUEUE_SIZE);

void TaskQueue::runInThread()
{
    LogInfo("TaskQueue: runInThread ...");
    while (!isStop())
    {
        swap();
        usleep(10000);
    }
}
void TaskQueue::swap()
{
    unique_lock<mutex> locker(queueMutex);
    if (!empty() && (m_processPoolStatus == P_EMPTY || m_processPoolStatus == P_PROCESSED))
    {
        size_t j = 0;
        for (size_t i = m_ut_front; i != m_ut_rear; modStepIn(i))
        {
            auto p = PopOneTask();
            if (p == nullptr)
            {
                LogError("taskPool should not be empty. rear=%ld, front=%ld, capacity=%ld, j=%ld, i=%ld",
                         front(), rear(), m_capacity, j, i);
                abort();
            }
            processPool[j] = p;
        }
        m_processPoolSize = j;
        m_processPoolStatus = P_FULL;
    }
}
TaskQueue::TaskQueue(size_t capacity) : m_capacity(capacity)
{
    taskPool = new Task *[m_capacity];
    processPool = new Task *[m_capacity];
    // g_pTaskQueue = this;
    
    static thread t(std::bind(&TaskQueue::runInThread, this));
}

void TaskQueue::putTaskBlock(Task *t)
{
    while (full())
    {
        usleep(10000);
    }
    unique_lock<mutex> locker(queueMutex);

    taskPool[rear()] = t;
    rearStepIn();
}

}

#endif