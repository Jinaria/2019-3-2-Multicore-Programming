#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>



int main(void){
    boost::asio::io_service io;

    boost::asio::deadline_timer t(io, boost::posix_time::seconds(2));
    t.wait();

    std::cout << "Hello, world!" << std::endl;

    return 0;
}