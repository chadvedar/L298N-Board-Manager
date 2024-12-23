#ifndef _SERIAL
#define _SERIAL

#include <boost/asio.hpp>
#include <fmt/core.h>

using namespace boost;

struct DataRecv{
    uint8_t data_recv[120] = {0};
    size_t bytes_read = 0;
};

class Serial{
    public:
        Serial(const char* _port, int _baudrate):
                port(_port),
                baudrate(_baudrate){};

        void init();
        void write(uint8_t* data_send, size_t data_size);
        DataRecv recv();
        DataRecv recvWithTimeout(uint16_t timeout);

        const char* port;
        int baudrate;
        bool isConnect = false;

    private:
        asio::io_context io;
        std::unique_ptr<asio::serial_port> serial;
        std::unique_ptr<asio::steady_timer > timer_;
};

#endif