#include <stdio.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define MAX_NUM_THREAD  16
#define MAX_NUM_WORK    100000

// list node structure
struct Node {
    int key;
    Node* next;
    pthread_mutex_t mutex;
};

// linked list
Node* head;
Node* tail;

// workload data
struct Work {
    char type;
    int value;
};

Work thread_work[MAX_NUM_THREAD][MAX_NUM_WORK];
int num_work;

// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// initialize linked list
void list_init(Node** head, Node** tail) {
    *head = (Node*)malloc(sizeof(Node));
    (*head)->key = INT_MIN;
    pthread_mutex_init(&(*head)->mutex, NULL);
    *tail = (Node*)malloc(sizeof(Node));
    (*tail)->key = INT_MAX;
    pthread_mutex_init(&(*tail)->mutex, NULL);

    (*head)->next = *tail;
    (*tail)->next = NULL;
}

bool list_validate(Node* prev, Node* cur){
    Node* it = head;
    while(it->key <= prev->key){
        if(it == prev){
            return (it->next == cur);
        }
        it = it->next;
    }
    return false;
}

// add key into the list
bool list_add(int key) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->key = key;

    pthread_mutex_init(&node->mutex, NULL);

    Node* prev;
    Node* cur;

    while(1){
        prev = head;
        cur = head->next;
        while(cur->key < key){
            prev = cur;
            cur = cur->next;
        }
        pthread_mutex_lock(&prev->mutex);
        pthread_mutex_lock(&cur->mutex);

        if(!list_validate(prev, cur)){
            pthread_mutex_unlock(&prev->mutex);
            pthread_mutex_unlock(&cur->mutex);
        }
        else{
            if(cur->key == key){
                pthread_mutex_unlock(&prev->mutex);
                pthread_mutex_unlock(&cur->mutex);
                return false;
            }
            node->next = cur;
            prev->next = node;

            pthread_mutex_unlock(&prev->mutex);
            pthread_mutex_unlock(&cur->mutex);
            break;
        }
    }
    return true;
}

// remove key from the list
bool list_remove(int key) {
    Node* prev;
    Node* cur;

    while(1){
        prev = head;
        cur = head->next;
        while(cur->key < key){
            prev = cur;
            cur = cur->next;
        }
        pthread_mutex_lock(&prev->mutex);
        pthread_mutex_lock(&cur->mutex);

        if(!list_validate(prev, cur)){
            pthread_mutex_unlock(&prev->mutex);
            pthread_mutex_unlock(&cur->mutex);
        }
        else{
            if(cur->key != key){
                pthread_mutex_unlock(&prev->mutex);
                pthread_mutex_unlock(&cur->mutex);
                return false;
            }
            prev->next = cur->next;
            pthread_mutex_unlock(&prev->mutex);
            pthread_mutex_unlock(&cur->mutex);


            break;
        }
    }

    return true;
}

// check whether a key is in the list
bool list_contains(int key) {
    bool is_contain = false;

    while(1){
        Node* prev = head;
        Node* cur = head->next;
        while(cur->key < key){
            prev = cur;
            cur = cur->next;
        }
        pthread_mutex_lock(&prev->mutex);
        pthread_mutex_lock(&cur->mutex);

        if(!list_validate(prev, cur)){
            pthread_mutex_unlock(&prev->mutex);
            pthread_mutex_unlock(&cur->mutex);
        }
        else{
            if(cur->key == key)
                is_contain = true;
            pthread_mutex_lock(&prev->mutex);
            pthread_mutex_lock(&cur->mutex);
            return is_contain;
        }
    }
}

// do workloads for each threads
void* ThreadFunc(void* args) {
    long tid = (long)args;

    for (int i = 0; i < num_work; i++) {
        switch (thread_work[tid][i].type) {
            case 'A':
                list_add(thread_work[tid][i].value);
                break;
            case 'R':
                list_remove(thread_work[tid][i].value);
                break;
            case 'C':
                list_contains(thread_work[tid][i].value);
                break;
            default:
                break;
        }
    }

    return NULL;
}

int main(void) {
    int num_thread;

    // input workloads
    FILE* fp = fopen("workload.txt", "r");

    fscanf(fp, "%d %d", &num_thread, &num_work);

    int thread_num;
    char work_type;
    int value;
    for (int i = 0; i < num_thread; i++) {
        fscanf(fp, "%d ", &thread_num);

        for (int j = 0; j < num_work; j++) {
            fscanf(fp, "%c %d ", &work_type, &value);

            thread_work[i][j].type = work_type;
            thread_work[i][j].value = value;
        }
    }

    fclose(fp);

    // set random seed
    srand(time(NULL));

    // initialize list
    list_init(&head, &tail);

    pthread_t threads[MAX_NUM_THREAD];

    // create threads
    for (long i = 0; i < num_thread; i++) {
        if (pthread_create(&threads[i], NULL, ThreadFunc, (void*)i) < 0) {
            exit(0);
        }
    }

    // wait threads end
    for (long i = 0; i < num_thread; i++) {
        pthread_join(threads[i], NULL);
    }

    // validate list
    FILE* fp_result = fopen("result.txt", "r");
    int size_result;
    int value_result;

    fscanf(fp_result, "%d", &size_result);

    bool is_valid = true;
    Node* it = head->next;
    for (int i = 0; i < size_result; i++) {
        if (it == tail) {
            is_valid = false;
            break;
        }
        fscanf(fp_result, "%d", &value_result);
        if (it->key != value_result) {
            is_valid = false;
            break;
        }
        it = it->next;
    }
    if (it != tail) {
        is_valid = false;
    }

    fclose(fp_result);

    if (is_valid) {
        printf("correct!\n");
    } else {
        printf("incorrect!\n");
    }

    return 0;
}

