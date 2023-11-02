#pragma once
#ifdef ECALL_BATCH

#include <unistd.h>

using namespace std;
namespace taskpool {

typedef enum 
{
    SAHE_ADD, // cipher + cipher
    SAHE_MUL, // cipher * plain
    SAHE_SUB, // cipher - cipher
    SAHE_NEG, // - cipher

    SMHE_MUL,  // cipher * cipher
    SMHE_MULP, // cipher * plain
    SMHE_DIV,  // cipher / cipher
    SMHE_POW,  // pow(cipher,n)
    SMHE_INV,  // 1 / cipher

    ORE,
    DET,
    SUM,
} OpType;

typedef enum
{
    T_PENDING,
    T_PROCESSING,
    T_PROCESSED
} TaskStatus;

typedef enum
{
    P_EMPTY,
    P_FULL,
    P_PROCESSING,
    P_PROCESSED
} PoolStatus;

struct Task
{
    // char* paramAES1;
    // char* paramAES2;
    char* paramAES1 = nullptr;
    char* paramAES2 = nullptr;
    char* res = nullptr;
    OpType type;
    TaskStatus status = T_PENDING;
    // Task(const Task&& r) {
    //     paramAES1 = move(r.paramAES1);
    //     paramAES2 = move(r.paramAES2);
    //     res = move(res);
    //     type = r.type;
    //     status = r.status;
    // }
    // Task(const Task &r);
    // Task();
    // Task& operator=(const Task &);
};

struct TaskQueue {
    size_t m_capacity = 0;
    size_t m_ut_rear = 0;
    size_t m_ut_front = 0;

    bool m_stop = false;
    PoolStatus m_processPoolStatus = P_EMPTY;
    Task **taskPool = nullptr;
    Task **processPool = nullptr;
    size_t m_processPoolSize = 0;

    ~TaskQueue() {
        stop();
    }
    void stop() {
        m_stop = true;
    }
    bool isStop() const
    {
        return m_stop;
    }
    size_t length() const {
        return (m_ut_rear - m_ut_front + m_capacity) % m_capacity;
    }
    bool empty() const {
        return length() == 0;
    }
    bool full() const
    {
        return length() == m_capacity;
    }

    bool readable() const { // 队列中是否还有未处理的Task
        return (m_ut_rear - m_ut_front + m_capacity) % m_capacity;
    }

    bool processed() const {
        return m_processPoolStatus == P_PROCESSED;
    }
    size_t front() const {
        return m_ut_front;
    }
    size_t rear() const
    {
        return m_ut_rear;
    }

    Task* getOneTask() {
        if(empty()) {
            return nullptr;
        }
    }
    void modStepIn(size_t& t) {
        t = (t + 1) % m_capacity;
    }
    void rearStepIn() {
        modStepIn(m_ut_rear);
    }
    void frontStepIn()
    {
        modStepIn(m_ut_front);
    }
    Task* PopOneTask() {
        if (empty())
        {
            return nullptr;
        }
        Task* p = taskPool[front()];
        frontStepIn();
        return p;
    }

#ifndef ENCLAVE
    void runInThread();
    void swap();
    TaskQueue(size_t capacity);
    void putTaskBlock(Task *t);
#endif
};

#ifndef ENCLAVE
extern taskpool::TaskQueue g_taskQueue;
#endif
}//namspace
#endif