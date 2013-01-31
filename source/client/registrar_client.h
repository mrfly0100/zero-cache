#ifndef REGISTRAR_CLIENT_H
#define REGISTRAR_CLIENT_H

#include <string>
#include <map>

#include "debug.h"
#include "types_zcache.h"
#include "socket.h"
#include "client_list.h"
#include "client_base.h"

namespace zero_cache
{

class Client;

class RegistrarClient : public ClientBase
{
public:
    RegistrarClient(const char* log_file, Connection connection, const SocketType type);
    virtual ~RegistrarClient() {};

    virtual void SetQueueSize(const int size);
    virtual void SetHost(const std::string host);

    void WriteData(const std::string key, const void* data, const size_t size);
    void* ReadData(const std::string key);
    KeyArray GetKeys();

private:
    mutable ClientList clients_;

    void AddKey(const std::string& key);
    Client* GetClient(const std::string& key);
    port_t SendPortRequest(const std::string& key);
};

}

#endif
