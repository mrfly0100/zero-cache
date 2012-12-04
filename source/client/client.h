#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <czmq.h>

#include "debug.h"
#include "types.h"

namespace zero_cache
{

class Client : protected Debug
{
public:
    Client(std::string log_file = "", std::string connection = "tcp://localhost:5570");
    virtual ~Client();

    void WriteData(std::string key, void* data, size_t size);
    void* ReadData(std::string key);

    void SetQueueSize(int size);

private:
    zctx_t* context_;
    void* socket_;

    void SendReadRequest(std::string key);
    void* ReceiveReadAnswer();

    DISALLOW_COPY_AND_ASSIGN(Client)
};

}

#endif
