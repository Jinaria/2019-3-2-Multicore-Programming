#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

#define NUM_THREAD  10

int thread_ret[NUM_THREAD];
int thread_work[NUM_THREAD];
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; 
// pthread_mutex_t m[NUM_THREAD];
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

int range_start;
int range_end;

int cond = NUM_THREAD;

bool IsPrime(int n) {
    if (n < 2) {
        return false;
    }

    for (int i = 2; i <= sqrt(n); i++) {
        if (n % i == 0) {
            return false;
        }
    }
    return true;
}

void* ThreadFunc(void* arg) {
    long tid = (long)arg;
    
    while(1){
        // printf("in thread%d while\n", tid);
        // printf("cond %d, thread_work %d\n", cond, thread_work[tid]);
        while(cond >= NUM_THREAD || thread_work[tid]){
            pthread_mutex_lock(&m);
            pthread_cond_wait(&c, &m);  
            pthread_mutex_unlock(&m);
        } 
        // printf("cond %d, thread_work %d\n", cond, thread_work[tid]);
        if(range_start == -1) break;
        
        int start = range_start + ((range_end - range_start + 1) / NUM_THREAD) * tid;
        int end = range_start + ((range_end - range_start + 1) / NUM_THREAD) * (tid+1);
        if (tid == NUM_THREAD - 1) {
            end = range_end + 1;
        }
        
        long cnt_prime = 0;
        for (int i = start; i < end; i++) {
            if (IsPrime(i)) {
                cnt_prime++;
            }
        }

        thread_ret[tid] = cnt_prime;
        thread_work[tid] = 1;
        __sync_fetch_and_add(&cond, 1);
        
    }
    // printf("thread%d exit\n", tid);
    // printf("cond %d\n", cond);
    thread_ret[tid] = 0;
    // Split range for this thread
        
    return NULL;
}

int main(void) {
    pthread_t threads[NUM_THREAD];
    // Create threads to work
    for(long i = 0; i < NUM_THREAD; i++){
        // m[i] = PTHREAD_MUTEX_INITIALIZER;
        if(pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0){
            printf("pthread_create error!\n");
            return 0;
        }
    }
    
    while (1) {
        // Input range
        // printf("in while\n");
        scanf("%d", &range_start);
        if (range_start == -1) {
            // exit(0);
            // return 0;
            cond = 0;
            pthread_mutex_lock(&m);
            pthread_cond_broadcast(&c);
            pthread_mutex_unlock(&m);
            break;
        }
        scanf("%d", &range_end);

        cond = 0;
        pthread_mutex_lock(&m);
        pthread_cond_broadcast(&c);
        pthread_mutex_unlock(&m);
        while(cond < NUM_THREAD) pthread_yield();

        // Collect results
        int cnt_prime = 0;
        for (int i = 0; i < NUM_THREAD; i++) {
            cnt_prime += thread_ret[i];
        }
        printf("number of prime: %d\n", cnt_prime);
        memset(thread_work, 0, sizeof(thread_work));
        
    }
    // printf("main exit\n");
    // printf("cond %d\n", cond);

    // Wait threads end
    for (int i = 0; i < NUM_THREAD; i++) {

        pthread_join(threads[i], NULL);
        // printf("joined %d\n", i);
    }
 
    return 0;
}

