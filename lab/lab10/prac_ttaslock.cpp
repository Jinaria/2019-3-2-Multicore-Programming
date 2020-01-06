#include <stdio.h>
#include <pthread.h>

#define NUM_TRHEAD 16
#define NUM_WORK 1000000

int cnt_global;
int gap[128];
int object_ttas;

void lock(int* lock_object){
    while(1){
        while(*lock_object == 1){}
        if(__sync_lock_test_and_set(lock_object, 1) == 0) {
            break;
        }
    }
}

void unlock(int* lock_object){
    __sync_synchronize();
    *lock_object = 0;
}

void* thread_work(void* argx){
    for(int i = 0; i < NUM_WORK; i++){
        lock(&object_ttas);
        cnt_global++;
        unlock(&object_ttas);
    }
}

int main(void){
    pthread_t threads[NUM_TRHEAD];
    
    for(int i = 0; i < NUM_TRHEAD; i++){
        pthread_create(&threads[i], NULL, thread_work, NULL);
    }

    for(int i = 0; i < NUM_TRHEAD; i++){
        pthread_join(threads[i], NULL);
    }

    printf("cnt_global: %d\n", cnt_global);

    return 0;
}