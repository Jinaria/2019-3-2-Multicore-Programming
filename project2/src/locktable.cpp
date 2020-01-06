#include "locktable.hpp"

locktable::locktable(int N, ull R):threadNum(N), size(R){
    for(ull i = 0; i < size; i++)
        locks.emplace_back();
    info.assign(threadNum + 1, list<pair<ull, bool> >());
}

locktable::~locktable(){
}

// void locktable::printLocktable(){
//     for(ull i = 0; i < size; i++){
//         printf("record %d : ", i);
//         for(auto j = locks[i].waiting.begin(); j != locks[i].waiting.end(); j++){
//             printf("%d, %d, %d->", j->tid, j->isRead, j->isLocked);
//         }
//         printf("\n");
//     }
//     printf("\n");
// }

// void locktable::printInfo(){
//     for(int i = 1; i <= threadNum; i++){
//         printf("thread %d : ", i);
//         for(auto j = info[i].begin(); j != info[i].end(); j++){
//             printf("%d, %d ->", j->first, j->second);
//         }
//         printf("\n");
//     }
//     printf("\n");
// }

bool locktable::detectDeadlock(ull record, int tid, int dst){

    auto k = locks[record].waiting.rbegin();
    for(; k != locks[record].waiting.rend(); k++){
        if(k->tid == tid) break;
    }

    for(; k != locks[record].waiting.rend(); k++){
        if(k->tid == dst) return true;
        for(auto j = info[k->tid].begin(); j != info[k->tid].end(); j++){
            if(j->second) continue;
            j->second = true;
            if(detectDeadlock(j->first, k->tid, dst)){
                return true;
            }
        }
    }

    return false;
}

bool locktable::needDetect(ull i, bool isRead){
    auto iter = locks[i].waiting.rbegin();
    if(iter == locks[i].waiting.rend()) return false;
    if(isRead && iter->isRead){
        return !iter->isLocked;
    }
    else{
        return true;
    }
}

bool locktable::readlockRecord(int tid, ull i, mutex *gM){
    if(needDetect(i, true)){
        
        int k = locks[i].waiting.rbegin()->tid;
        for(int a = 1; a <= threadNum; a++){
            for(auto iter = info[a].begin(); iter != info[a].end(); iter++){
                if(a == k && iter->first == i){
                    iter->second = true;
                }
                else{
                    iter->second = false;
                }
            }
        }
        // printInfo();
        if(detectDeadlock(i, k, tid)){
            return false;
        }
    }
    info[tid].emplace_back(i, false);
    locks[i].readLock(tid, gM);
    return true;
}

bool locktable::writelockRecord(int tid, ull i, mutex *gM){
    if(needDetect(i, false)){
        
        
        int k = locks[i].waiting.rbegin()->tid;
        for(int a = 1; a <= threadNum; a++){
            for(auto iter = info[a].begin(); iter != info[a].end(); iter++){
                if(a == k && iter->first == i){
                    iter->second = true;
                }
                else{
                    iter->second = false;
                }
            }
        }
        // printInfo();
        if(detectDeadlock(i, k, tid)){
            return false;
        }
    }
    info[tid].emplace_back(i, false);
    locks[i].writeLock(tid, gM);
    return true;
}

bool locktable::readUnlockRecord(int tid, ull i){
    info[tid].pop_front();
    locks[i].readUnlock(tid);
    
    return true;
}

bool locktable::writeUnlockRecord(int tid, ull i){
    info[tid].pop_front();
    locks[i].writeUnlock(tid);
    
    return true;
}