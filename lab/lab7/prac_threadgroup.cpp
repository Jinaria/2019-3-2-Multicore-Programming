#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <cmath>
#include <cstdio>

#define NUM_THREAD_IN_POOL  10

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

boost::asio::io_service producer;
boost::asio::io_service consumer;
boost::thread_group producers;
boost::thread_group consumers;
boost::asio::io_service::work* pwork;
boost::asio::io_service::work* cwork;


void consume(int seq, int result, int start, int end){
    printf("(%d)number of primes in %d ~ %d is %d\n", seq, start, end, result);
}

void produce(int seq, int range_start, int range_end){
    int result = 0;
    for(int i = range_start; i <= range_end; ++i){
        if(IsPrime(i)){
            result++;
        }
    }

    consumer.post(boost::bind(consume, seq, result, range_start, range_end));
}

int main(void) {
    pwork = new boost::asio::io_service::work(producer);    
    cwork = new boost::asio::io_service::work(consumer);

    for (int i = 0; i < NUM_THREAD_IN_POOL - 1; i++) {
        producers.create_thread(boost::bind(
                    &boost::asio::io_service::run, &producer));
        
    }
    consumers.create_thread(boost::bind(
                    &boost::asio::io_service::run, &consumer));

    int range_start;
    int range_end;
    int seq = 0;
    scanf("%d %d", &range_start, &range_end);
    while(range_start != -1){
        producer.post(boost::bind(produce, seq, range_start, range_end));
        seq++;
        scanf("%d %d", &range_start, &range_end);
        // printf("%d %d %d\n", seq, range_start, range_end);
    }

    delete pwork;
    producers.join_all();
    delete cwork;
    consumers.join_all();
    producer.stop();
    consumer.stop();

    return 0;
}