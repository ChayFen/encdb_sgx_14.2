#ifdef ECALL_BATCH

#pragma once
#include "TaskQueue.h"
#include <pthread.h>

namespace taskpool
{

class TaskPoolLoop
{
    pthread_t m_thread;
    TaskQueue *m_pTaskQueue = nullptr;

public:
    TaskPoolLoop(TaskQueue *pTaskQueue) : m_pTaskQueue(pTaskQueue)
    {
    }


    void processTask(taskpool::Task *task);

    void processTaskPool();

    void TaskQueueMainLoop();
    void loop();
};
} // namespace

#endif