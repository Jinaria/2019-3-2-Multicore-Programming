# Reader - Writer Lock
## Outline
Goal of this project is to implement reader writer lock about special case.
## My blueprint
I designed the rwlock as a lock that kept spinning until its turn came around. 
Readlock wakes up when its position change to first or when readlock that right before it on the list wakes up. 
Writelock wakes up only when it comes to the front of list. When Reader or Writer lock is released, it leaves the list. 
Database has records and lock table and global lock. 
Lock table has locks the same size of records, information that records for each thread. 
Deadlock detection will operate when lock must wait, and find cycle for current thread. 
## Specific implementation
### My file
`globals.hpp`

This file has header files that I need and constant value. It has typedef unsigned long long as ull.

`database.hpp`, `locktable.hpp`, `rwlock.hpp`

These files have definition of each class. rwlock class have lockInfo class as a member variable so rwlock.hpp have definition of that class.

`database.cpp`

Database class have member variable like record size, number of thread, number of execution, global lock to protect locktable, current commit id, and record table and lock table for each record. 

    database(int N, ull R, ull E);
Constructor of database class. It initalize each member variable. N is number of thread, R is size of records, E is number of execution.

    ~database();
Destructor of database class. It delete allocated variable.

    int getCommitId();
Return current commit id.

    bool transaction(int tid, ull i, ull j, ull k, ofstream &fs);
Call by main function. tid is thread id,  fs is out file stream for each thread. i, j, k are record number. It perform each step(2 ~ 16) of assignment. Acquire global lock and acquire readlock for record i, and release global lock and get record i. And about other two record number, same operation will operate only different not read but write. For each acquiring lock, if it fail acquiring, it operate rollback and return false. When every step to 13 have done, database starts commit. Acquiring global lock, release all read/write lock and increase commit id and append commit log into each thread log file. And release global lock and return true.

`locktable.cpp`


    locktable(int N, ull R);
Constructor of locktable class. N is number of thread, R is size of records. Initialize list of lock, and initialize list of thread info that has current record.

    bool needDetect(ull i, bool isRead);
It check whether deadlock detection is required. i is record number and isRead is whether it is readlock or writelock. If isRead is true and front of this lock is read lock, it return whether front lock is blocked. In other case, it return true.

    bool detectDeadlock(ull record, int tid, int dst);
Detect deadlock. record is current record, tid is current thread id, and dst is destination thread id. First, find iterator of current tid in waiting list. And from there to begin of list, if current iterator's tid is like dst, return true. If not, call detectDeadlock about all record of tid thread had. 

    bool readlockRecord(int tid, ull i, mutex *gM);
First, it check whether deadlock detection is required. If true, it initialize thread info and detect deadlock. If detected, return false. Or not, call readlock about record i. 

    bool writelockRecord(int tid, ull i, mutex *gM);
It perform same operation to readlockRecord function. Only different is call write lock about record i.

    bool readUnlock(int tid, ull i);
Modify thread info, and call readUnlock function of record i;

    bool writeUnlock(int tid, ull i);
Modify thread info, and call writeUnlock function of record j.

`rwlock.cpp`


    rwlock();
Constructor of rwlock class. This class has list of waiting. In waiting, it has state of lock, tid and isRead and isLocked. If isLocked is true, this lock is acquired.

    void readLock(int tid, mutex *gM);
If it is first of list or list only has readlock, lock is acquired. Or not, put in waiting of record, and spin until when this lock become first of waiting or read lock that front of this lock is acquired. When spinning, if it don't meet the condition, unlock gM(global lock) and yield another thread. And if it get turn, global lock again and check condition.

    void writeLock(int tid, mutex *gM);
If it is first of list, lock is acquired. Or not put in waiting of record, and spin until when this lock become first of waiting. When spinning, if it don't meet the condition, unlock gM(global lock) and yield another thread. And if it get turn, global lock again and check condition.

    void readUnlock(int tid);
Find readlock of tid in waiting, and erase it.

    void writeUnlock(int tid);
Find writelock of tid in waiting, and erase it.

`main.cpp`


    void pickRandNum(default_random_engine &g, uniform_int_distribution<unsigned long long> &d, ull *i, ull *j, ull *k);
Pick random number i. Pick second number and compare i. If same, pick again. Or not put j. Pick third number and compare i and j. If same i or j, pick again. Or not put k.

    void work(database *db, int tid, ofstream &fs);
This function is thread function. Repeat to send to database transaction, and transaction return false, repeat same value. 

    in main function
If argument number is less than 4, print error and exit. Or not, change arguments to N, R, E and create thread for transaction.
And wait until when all thread is joined.