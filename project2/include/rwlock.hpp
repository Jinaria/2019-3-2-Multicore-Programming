#ifndef RWLOCK_HPP__
#define RWLOCK_HPP__

#include "globals.hpp"

#define READ 0
#define WRITE 1


class infoLock{
public:
    bool isRead;
    bool isLocked;
    int tid;

    infoLock(bool isRead, bool isLocked, int tid):isRead(isRead),isLocked(isLocked),tid(tid){

    }
};

class rwlock{
public:
    list<infoLock> waiting;

    rwlock();

    void readLock(int tid, mutex *gM);
    void writeLock(int tid, mutex *gM);
    void readUnlock(int tid);
    void writeUnlock(int tid);
};

#endif