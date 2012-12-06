#ifndef REGISTRAR_CLIENT_H
#define REGISTRAR_CLIENT_H

#include <string>
#include <map>
#include <list>
#include <czmq.h>

#include "debug.h"
#include "types_zcache.h"

namespace zero_cache
{

class Client;

class RegistrarClient : protected Debug
{
private:
    typedef std::map<std::string, Client*> KeyClient;
    typedef std::list<Client> ClientList;

public:
    RegistrarClient(std::string log_file = "", std::string connection = "tcp://localhost:5570");
    virtual ~RegistrarClient();

    void WriteData(std::string key, void* data, size_t size);
    void* ReadData(std::string key);

private:
    zctx_t* context_;
    void* socket_;
    KeyClient clients_;
    ClientList client_repository_;

    void AddKey(std::string key);
    Client* GetClient(std::string key);
    std::string ReceiveAnswer(zframe_t* key);

    DISALLOW_COPY_AND_ASSIGN(RegistrarClient);
};

}

#endif
