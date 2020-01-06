#ifndef LOCKTABLE_HPP__
#define LOCKTABLE_HPP__

#include "globals.hpp"
#include "rwlock.hpp"

typedef vector<list<pair<ull, bool> > > threadInfo;

class locktable{
private:
    ull size;
    int threadNum;
    vector<rwlock> locks;
    threadInfo info;

    void printLocktable();
    void printInfo();
    bool detectDeadlock(ull record, int tid, int dst);
    bool needDetect(ull i, bool isRead);

public:
    locktable(int N, ull R);
    ~locktable();
    
    bool readlockRecord(int tid, ull i, mutex *gM);
    bool writelockRecord(int tid, ull i, mutex *gM);
    bool readUnlockRecord(int tid, ull i);
    bool writeUnlockRecord(int tid, ull i);
};

#endif