#pragma once
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <wait.h>
#include "LogBase.h"


#define FPATH  "/tmp/encryptsql/file1.lock"
namespace util {

class FileMutexLocker {
public:
    FileMutexLocker() {
        fd = open(FPATH, O_RDWR | O_CREAT | O_TRUNC, 0644);
        if(fd == -1) {
            LogError("can not open %s", FPATH);
            abort();
        }
        LogInfo("Opened For %s", FPATH);
    }
    ~FileMutexLocker() {
        close(fd);
        remove(FPATH); 
    }
    bool lock() {
        return lockf(fd, F_TLOCK, 0) == 0;
    }
    void unlock() {
        lockf(fd, F_ULOCK, 0) ;
    }

private:
    
    int fd; 
};



}
