#pragma once
#include <pthread.h>
#include <sys/shm.h> 
#include <sys/mman.h> 
#include "LogBase.h"
#include <string.h>
#include <mutex>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define MMAP_BUFF_SIZE 4096

namespace util {

#define SMPATH  "/tmp/encryptsql/enc.mmap"

struct mmap_t {
    bool EPCPaging;
    int epcThread;
    pthread_mutex_t mutex;
    // pthread_mutexattr_t attr;
};

class SharedMemory {
public:
    SharedMemory() {
        mkdir("/tmp/encryptsql/", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        int shmid = shmget((key_t)98652, MMAP_BUFF_SIZE, 0666|IPC_CREAT);
        if (shmid == -1)
        {
            LogError("shmget failed\n");
            exit(EXIT_FAILURE);
        }
    
        // 将共享内存连接到当前的进程地址空间
        _handle = (mmap_t*)shmat(shmid, (void *)0, 0);
        if ((void*)_handle == (void *)-1)
        {
            LogError("shmat failed\n");
            exit(EXIT_FAILURE);
        }
    
        LogInfo("Memory attched at %lX\n", (uint64_t)_handle);
        
        
        // _fd = open(SMPATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
        // write(_fd, "0329", 4);
        // lseek(_fd,MMAP_BUFF_SIZE,SEEK_SET);
        // if ((_handle = (mmap_t*)mmap(NULL, MMAP_BUFF_SIZE, PROT_READ |   
        //                     PROT_WRITE, MAP_SHARED, _fd, 0)) == (void *)-1) {  
        //         printf("create memory map failed");
        //         printf(strerror(errno));
        //         exit(-1);
        //     }
        //     // memcpy((void*)_handle->flag, (const void*)"4631", 4);
        //     // _handle->flag = "4631";  
        //     close(_fd);
    }
    mmap_t* getHandle() {
        return _handle;
    };
    ~SharedMemory() {
        shmdt((void*)_handle);
        // munmap((void*)_handle, MMAP_BUFF_SIZE); 
        // LogInfo("decs shared mm");
    }

private:
    int _fd;
    mmap_t* _handle;

};

class ShmMutex {
public:
    ShmMutex() {};
    void setHandle(mmap_t* mmp) {
        _handle = &mmp->mutex;
    }
    ShmMutex(mmap_t* shm);
    virtual ~ShmMutex() ;
    void setHandle();
    void lock();
    void unlock();
    bool tryLock();
private:
    pthread_mutex_t* _handle;
};

}
