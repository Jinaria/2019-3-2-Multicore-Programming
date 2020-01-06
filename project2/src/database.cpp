#include "database.hpp"

database::database(int N, ull R, ull E):size(R),txnNum(E), threadNum(N){
    globalMutex = new mutex;
    commit_id = 0;
    records.assign(R, INIT_VALUE);
    lt = new locktable(N, R);
}

database::~database(){
    delete lt;
    delete globalMutex;
}

ull database::getCommitId(){
    return commit_id;
}

bool database::transaction(int tid, ull i, ull j, ull k, ofstream &fs){
    
    // locking phase
    globalMutex->lock();
    if(!lt->readlockRecord(tid, i, globalMutex)){
        globalMutex->unlock();
        return false;
    }
    globalMutex->unlock();
    long long vi = records[i];

    globalMutex->lock();
    if(!lt->writelockRecord(tid, j, globalMutex)){
        lt->readUnlockRecord(tid, i);
        globalMutex->unlock();
        return false;
    }
    globalMutex->unlock();
    long long vj = records[j];
    records[j] += vi + 1;

    globalMutex->lock();
    if(!lt->writelockRecord(tid, k, globalMutex)){
        records[j] = vj;
        lt->readUnlockRecord(tid, i);
        lt->writeUnlockRecord(tid, j);
        globalMutex->unlock();
        return false;
    }
    globalMutex->unlock();
    long long vk = records[k];
    records[k] -= vi;

    // release phase
    globalMutex->lock();
    lt->readUnlockRecord(tid, i);
    lt->writeUnlockRecord(tid, j);
    lt->writeUnlockRecord(tid, k);
    commit_id++;
    if(commit_id > txnNum){
        
        records[j] = vj;
        records[k] = vk;
        globalMutex->unlock();
        return false;
    }
    fs << commit_id << " " << i + 1  << " " << j + 1  << " " << k + 1 << " " 
    << records[i] << " " << records[j] << " " << records[k] << '\n';
    globalMutex->unlock();
    return true;

}