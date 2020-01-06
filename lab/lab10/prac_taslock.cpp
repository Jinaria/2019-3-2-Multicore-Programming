#include <stdio.h>
#include <pthread.h>

#define NUM_TRHEAD 16
#define NUM_WORK 1000000

int cnt_global;
// to allocate cnt_global & object_tas in different cache line
int gap[128];
int object_tas;

void lock(int* lock_object){
    while(__sync_lock_test_and_set(lock_object, 1) == 1) {}
}

void unlock(int* lock_object){
    __sync_synchronize();
    *lock_object = 0;
}

void* thread_work(void* argx){
    for(int i = 0; i < NUM_WORK; i++){
        lock(&object_tas);
        cnt_global++;
        unlock(&object_tas);
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