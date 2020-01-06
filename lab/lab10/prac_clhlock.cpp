#include <stdio.h>
#include <pthread.h>

#define NUM_TRHEAD 16
#define NUM_WORK 1000000

int cnt_global;
int gap[128];
bool* tail;


void lock(bool* flag){
    bool * before = __sync_lock_test_and_set(&tail, flag);
    while(*before == true){
        pthread_yield();
    }
    delete before;
}

void unlock(bool* flag){
    __sync_synchronize();
    *flag = false;
}

void* thread_work(void* argx){
    bool *flag;
    for(int i = 0; i < NUM_WORK; i++){
        flag = new bool(true);
        lock(flag);
        cnt_global++;
        unlock(flag);
    }
}

int main(void){
    pthread_t threads[NUM_TRHEAD];
    tail = new bool;
    
    for(int i = 0; i < NUM_TRHEAD; i++){
        pthread_create(&threads[i], NULL, thread_work, NULL);
    }

    for(int i = 0; i < NUM_TRHEAD; i++){
        pthread_join(threads[i], NULL);
    }

    printf("cnt_global: %d\n", cnt_global);
    delete tail;
    return 0;
}