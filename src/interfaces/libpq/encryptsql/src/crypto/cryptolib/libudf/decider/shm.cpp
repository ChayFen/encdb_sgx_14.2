
#include "shm.h"
#include <stdlib.h>
#include <exception>

#include <iostream>
#include <mutex>
#include "FileMutex.h"
using namespace std;

namespace util {

ShmMutex::ShmMutex(mmap_t* shm) {
    _handle = &shm->mutex;
    pthread_mutexattr_t attr;
    // pthread_mutexattr_t* pattr = &shm->attr;
    pthread_mutexattr_t* pattr = &attr;
    int ret;
    if((ret = pthread_mutexattr_init(pattr)) != 0 ) {
        LogError("failed to init attr\n");
    }
    if((ret = pthread_mutexattr_setpshared(pattr, PTHREAD_PROCESS_SHARED)) != 0) {
        LogError("failed to set shared\n");
    }
    if((ret = pthread_mutex_init(_handle, pattr)) != 0) {
        LogError("Unable to create mutex\n");

    }
}
ShmMutex::~ShmMutex()
{
}   

void ShmMutex::lock()
{
    if (pthread_mutex_lock((pthread_mutex_t*)_handle) != 0) {
        LogError("tid:%d Unable to lock mutex ",gettid());
    }
}
void ShmMutex::unlock()
{
    if (pthread_mutex_unlock((pthread_mutex_t*)_handle) != 0) {
        LogError("Unable to unlock mutex");
    }
}
bool ShmMutex::tryLock()
{
    int tryResult = ::pthread_mutex_trylock((pthread_mutex_t*)_handle);
    if (tryResult != 0) {
        if (EBUSY == tryResult) return false;
    }
    return true;
}
}

// #include <stdlib.h>
// #include <stdio.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/file.h>
// #include <wait.h>
// #include <sys/mman.h>  
// #include <sys/stat.h>  
// #include <memory.h>


// using namespace util;
// int main()
// {
//     pid_t pid;
//     mmap_t* mm;
//     ShmMutex* pShmMutex;
//     pid = fork();
//     char *mapped, *mappedlock;

//     if (pid == 0) { /* 子进程 */ //write
//         SharedMemory shm;
//         mm = shm.getHandle();
//         int i = 0;
//         FileMutexLocker fileMutex;
//         if(!fileMutex.lock()) {
//             sleep(2);
//             pShmMutex = new ShmMutex();
//             pShmMutex->setHandle(mm);
//             printf("!write init lock done\n");
//         }
//         else {
//             pShmMutex = new ShmMutex(mm);
//             printf("write init lock done\n");
//         }
//         while (i < 10) {  
//             i++;
//             printf("wirter lock 1\n");
//             pShmMutex->lock();
//             printf("wirter lock 2\n");

//             mm->epcThread= i;
//             printf("wirter %d\n", i);
//             printf("wirter re 1\n");
//             pShmMutex->unlock();
//             printf("wirter re 2\n");
//             sleep(2);
//         }
//         printf("wirte exit\n");  
//     }
//     else { // read
//         SharedMemory shm;
//         mm = shm.getHandle();
//         FileMutexLocker fileMutex;
//         if(!fileMutex.lock()) {
//             sleep(2);
//             pShmMutex = new ShmMutex();
//             pShmMutex->setHandle(mm);
//             printf("!read init lock done\n");
//         }
//         else {
//             pShmMutex = new ShmMutex(mm);
//             printf("read init lock done\n");
//         }
//         int i = 10;
//         while (i --) {  
//             printf("read lock 1\n");
//             pShmMutex->lock();
//             printf("read lock 2\n");

//             printf("read %d\n", mm->epcThread);  
//             printf("read rease 1\n");
//             pShmMutex->unlock();
//             printf("read rease 2\n");
//             sleep(2);  
//         }  
//         printf("read exit\n");
//     }
//     wait(NULL);
//     // unlink(PATH);
//     exit(0);
// }