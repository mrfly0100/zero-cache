#include "registrar_client.h"

#include <string.h>
#include <stdlib.h>

#include "client.h"
#include "request.h"
#include "functions.h"

#define PRINTF Log
#include "speed_test.h"

using namespace std;
using namespace zero_cache;

static const long kReadAnswerTimeout = 10;

RegistrarClient::RegistrarClient(const char* log_file, Connection connection, SocketType type) :
    ClientBase(log_file, connection, type), clients_(connection, type)
{
    srand(time(NULL));

    SetHost(connection.GetHost());
}

KeyArray RegistrarClient::GetKeys()
{
    KeyArray keys;

    request_->SetCommand(kGetKeys);
    request_->Send(socket_);

    keys = ReceiveKeys();

    return keys;
}

KeyArray RegistrarClient::ReceiveKeys()
{
    if ( ! socket_.ReceiveMsg(kReadAnswerTimeout) )
        return KeyArray();

    zmq_msg_t keys_msg;
    socket_.PopMsg(keys_msg);

    KeyArray keys = MsgToKeyArray(keys_msg);

    zmq_msg_close(&keys_msg);

    return keys;
}

void RegistrarClient::WriteData(string key, void* data, size_t size)
{
    Log("RegistrarClient::WriteData() - key = %s data_size = %lu\n", key.c_str(), size);

    PRE_TIME_MEASURE("RegistrarClient::WriteData() ")

    GetClient(key)->WriteData(key, data, size);

    POST_TIME_MEASURE
}

void* RegistrarClient::ReadData(string key)
{
    Log("RegistrarClient::ReadData() - key = %s\n", key.c_str());

    PRE_TIME_MEASURE("RegistrarClient::ReadData() ")

    void* result = GetClient(key)->ReadData(key);

    POST_TIME_MEASURE

    return result;
}

Client* RegistrarClient::GetClient(string& key)
{
    AddKey(key);

    return clients_.GetClient(key);
}

void RegistrarClient::AddKey(string& key)
{
    if ( clients_.IsKeyExist(key) )
        return;

    port_t port = SendPortRequest(key);

    Log("RegistrarClient::AddKey() - add key = %s port = %lu\n", key.c_str(), port);

    clients_.AddKey(key, port);

    clients_.CreateClient(port);
}

port_t RegistrarClient::SendPortRequest(string& key)
{
    request_->SetCommand(kGetPort);
    request_->SetKey(key);

    port_t port = kErrorPort;

    while ( port == kErrorPort )
    {
        request_->Send(socket_);

        port = ReceivePort();

        if ( port == kErrorPort )
            usleep((rand() % 1000) * 100);
    }

    return port;
}

port_t RegistrarClient::ReceivePort()
{
    if ( ! answer_.Receive(socket_, kReadAnswerTimeout) )
        return kErrorPort;

    return answer_.GetPort();
}

void RegistrarClient::SetHost(string host)
{
    ClientBase::SetHost(host);

    clients_.SetHost(host);
}

void RegistrarClient::SetQueueSize(int size)
{
    clients_.SetQueueSize(size);
}
