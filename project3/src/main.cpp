#include "globals.hpp"

class snapValue{
public:
    int N;
    uint64_t label;
    int64_t value;
    int64_t *snap;
    snapValue():N(0), label(0), value(0), snap(NULL){}
    snapValue(int N, uint64_t label, int64_t value, int64_t *snap):N(N), label(label), value(value), snap(snap){}
    snapValue(snapValue& a):N(a.N), label(a.label), value(a.value){
        snap = new int64_t[N];
        for(int i = 0; i < N; ++i){
            snap[i] = a.snap[i];
        }
    }
    ~snapValue(){
        delete[] snap;
    }
};

typedef shared_ptr<snapValue> valuePtr;

// interface
class snapshot{
public:
    virtual void update(int tid, int64_t v) = 0;
    virtual int64_t* scan() = 0;
};

class WFSnapshot : public snapshot{
private:
    int N;
    valuePtr *a_table;

    valuePtr* collect(){
        valuePtr *copy = new valuePtr[N];
        for(int i = 0; i < N; ++i){
            copy[i] = a_table[i];
        }
        return copy;
    }
    
public:
    WFSnapshot(int N):N(N){
        a_table = new valuePtr[N];
        for(int i = 0; i < N; ++i){
            a_table[i] = make_shared<snapValue>(N, 0, 0, new int64_t[N]);
        }
    }
    ~WFSnapshot(){
        // delete[] a_table;
    }
    virtual int64_t* scan(){
        valuePtr *oldCopy;
        valuePtr *newCopy;
        vector<bool> moved(N);
        oldCopy = collect();
        while(true){
            newCopy = collect();
            bool flag = true;
            for(int j = 0; j < N; ++j){
                if(oldCopy[j]->label != newCopy[j]->label){
                    if(moved[j]){
                        delete[] newCopy;
                        int64_t * ret = new int64_t[N];
                        for(int i = 0; i < N; ++i)
                            ret[i] = oldCopy[j]->snap[i];
                        delete[] oldCopy;
                        return ret;
                    }
                    else{
                        moved[j] = true;
                        auto temp = oldCopy;
                        oldCopy = newCopy;
                        delete[] temp;
                        flag = false;
                        break;
                    }
                }
            }
            if(flag) break;
        }
        int64_t *result = new int64_t[N];
        for(int j = 0; j < N; ++j)
            result[j] = newCopy[j]->value;
        delete[] oldCopy;
        delete[] newCopy;
        
        return result;
    
    }
    virtual void update(int tid, int64_t v){
        int64_t *snap = scan();
        valuePtr oldValue(a_table[tid]);
        snapValue newValue(N, oldValue->label + 1, v, snap);
        // auto temp = a_table[tid].snap;
        *a_table[tid] = newValue;
        // if(temp != NULL) delete[] temp;
    }
    uint64_t labelNum(int tid){
        return a_table[tid]->label;
    }
};

WFSnapshot *global;
int flag = false;
int start, endt;
uint64_t throughput = 0;



void work(int tid){
    endt = time(NULL);
    while(true){
        if(endt - start >= 60) break;
        int value = rand();
        global->update(tid, (int64_t)value);
        endt = time(NULL);
    }
    int k = global->labelNum(tid);
    throughput = throughput + k;
}

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("execute with format ./run N\n");
        return 0;
    }
    srand((unsigned int)time(NULL));
    int N = stoi(argv[1]);
    global = new WFSnapshot(N);
    // printf("%d\n", N);

    vector<thread> workers;
    start = time(NULL);
    for(int i = 0; i < N; ++i){
        workers.emplace_back(work, i);
    }
    for(int i = 0; i < N; ++i){
        workers[i].join();
    }

    printf("throughput %llu\n", throughput);
    delete global;

    return 0;
}
