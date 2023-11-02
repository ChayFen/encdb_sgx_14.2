#ifdef ECALL_BATCH

#include "EnclaveTaskQueue.h"

#include "Enclave_t.h"


namespace taskpool{

    void TaskPoolLoop::processTask(taskpool::Task *task)
    {
        task->status = T_PROCESSING;
        switch (task->type)
        {
        case SAHE_ADD:
            // size_t outSize =
            ecall_sahe_add((void *)task->paramAES1, (void *)task->paramAES2, (void *)task->res, 0);

            break;

        default:
            break;
        }
        task->status = T_PROCESSED;
}

void TaskPoolLoop::processTaskPool()
{
    auto poolSize = m_pTaskQueue->m_processPoolSize;
    for (int i = 0; i < poolSize; ++i)
    {
        processTask(m_pTaskQueue->processPool[i]);
    }
}

void TaskPoolLoop::TaskQueueMainLoop()
{
    while (!m_pTaskQueue->m_stop)
    {
        if (m_pTaskQueue->m_processPoolStatus != P_FULL)
        {
            continue;
        }
        m_pTaskQueue->m_processPoolStatus = P_PROCESSING;
        processTaskPool();
        m_pTaskQueue->m_processPoolStatus = P_PROCESSED;
        ocall_usleep(1000); // TODO: 实现ocall_usleep
    }
}
void TaskPoolLoop::loop()
{
    typedef void *(*THREADFUNCPTR)(void *);
    pthread_create(&m_thread, NULL, (THREADFUNCPTR)&TaskPoolLoop::TaskQueueMainLoop, this);
    // TaskQueueMainLoop();
}
}; // namespace

#endif