#include "globals.hpp"
#include "database.hpp"

ull R, E;
int N;

void pickRandNum(default_random_engine &g, uniform_int_distribution<unsigned long long> &d, ull *i, ull *j, ull *k){
    *i = d(g);
    do {
        *j = d(g);
    } while(*j == *i);
    do {
        *k = d(g);
    } while(*k == *i || *k == *j);
}

void work(database *db, int tid, ofstream *fs){
    random_device rd;
    default_random_engine generator(rd());
    uniform_int_distribution<unsigned long long> distribution(0, R - 1);
    if(fs->is_open()){
        ull i, j, k;
        pickRandNum(generator, distribution, &i, &j, &k);
        while(db->getCommitId() < E){
            bool flag = db->transaction(tid, i, j, k, *fs);
            if (!flag) continue;
            pickRandNum(generator, distribution, &i, &j, &k);
        }
        fs->close();
    }
    else{
        cout << "fail to open log file " << to_string(tid) << endl;
    }
}
    

int main(int argc, char *argv[]){
    if(argc < 4){
        cout << "use ./exe_file N R E format\n";
        return 0;
    }
    N = atoi(argv[1]), R = atoll(argv[2]), E = atoll(argv[3]);
    database db(N, R, E);

    vector<thread> workers;
    vector<ofstream*> fs;
    for(int i = 0; i < N; i++){
        string outpath = "thread" + to_string(i + 1) + ".txt";
        fs.push_back(new ofstream(outpath, ios::trunc));
        workers.emplace_back(work, &db, i + 1, fs[i]);
    }
    for(int i = 0; i < N; i++){
        workers[i].join();
        delete fs[i];
    }
    return 0;
}