#ifndef DATABASE_HPP__
#define DATABASE_HPP__

#include "globals.hpp"
#include "locktable.hpp"

class database{
private:
    ull size;
    ull txnNum;
    int threadNum;
    mutex* globalMutex;
    ull commit_id;
    vector<long long> records;
    locktable *lt;
    
public:
    database(int N, ull R, ull E);
    ~database();

    ull getCommitId();
    bool transaction(int tid, ull i, ull j, ull k, ofstream &fs);
};

#endif