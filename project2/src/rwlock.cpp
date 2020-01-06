#include "rwlock.hpp"

rwlock::rwlock(){
}

void rwlock::readLock(int tid, mutex *gM){
    if(waiting.empty() || (waiting.back().isRead && waiting.back().isLocked)){
        waiting.emplace_back(true, true, tid);
    }
    else if(!waiting.back().isRead || !waiting.back().isLocked){
        waiting.emplace_back(true, false, tid);
        auto myIter = waiting.end();
        myIter--;
        while(true){
            if(myIter == waiting.begin()){
                myIter->isLocked = true;
                break;
            }
            myIter--;
            if(myIter->isRead && myIter->isLocked){
                myIter++;
                myIter->isLocked = true;
                break;
            }
            myIter++;
            gM->unlock();
            this_thread::yield();
            gM->lock();
        }
    }
}

void rwlock::writeLock(int tid, mutex *gM){

    if(waiting.empty()){
        waiting.emplace_back(false, true, tid);
    }
    else{
        waiting.emplace_back(false, false, tid);
        auto myIter = waiting.end();
        myIter--;
        while(myIter != waiting.begin()){
            myIter->isLocked = false;
            gM->unlock();
            this_thread::yield();
            gM->lock();
            myIter->isLocked = true;
        }
    }
}

void rwlock::readUnlock(int tid){

    auto myIter = waiting.begin();
    while(myIter->tid != tid) myIter++;

    waiting.erase(myIter);
}

void rwlock::writeUnlock(int tid){
    auto myIter = waiting.begin();
    while(myIter->tid != tid) myIter++;

    waiting.erase(myIter);
}