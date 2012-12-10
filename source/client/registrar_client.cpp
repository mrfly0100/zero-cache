#include "registrar_client.h"

#include "client.h"

#define STREAM Log()
#include "speed_test.h"

using namespace std;
using namespace zero_cache;

static const long kReadAnswerTimeout = 10;

RegistrarClient::RegistrarClient(string log_file, string connection) : Debug(log_file), queue_size_(10)
{
    srand(time(NULL));

    context_ = zctx_new();
    socket_ = zsocket_new(context_, ZMQ_DEALER);

    zsocket_connect(socket_, connection.c_str());
    zsocket_set_hwm(socket_, 1);
}

RegistrarClient::~RegistrarClient()
{
    zsocket_destroy(context_, socket_);
    zctx_destroy(&context_);
}

void RegistrarClient::WriteData(string key, void* data, size_t size)
{
    Log() << "RegistrarClient::WriteData() - key = " << key << " data_size = " << size << endl;

    PRE_TIME_MEASURE("RegistrarClient::WriteData")

    GetClient(key)->WriteData(key, data, size);

    POST_TIME_MEASURE
}

void* RegistrarClient::ReadData(string key)
{
    Log() << "RegistrarClient::ReadData() - key = " << key << endl;

    return GetClient(key)->ReadData(key);
}

Client* RegistrarClient::GetClient(string key)
{
    AddKey(key);

    return clients_[connections_[key]];
}

void RegistrarClient::AddKey(string key)
{
    if ( connections_.count(key) != 0 )
        return;

    string connection = "";
    zframe_t* key_frame;
    while ( connection.empty() )
    {
        key_frame = zframe_new(key.c_str(), key.size());

        zframe_send(&key_frame, socket_, ZFRAME_REUSE);

        connection = ReceiveAnswer(key_frame);

        usleep((rand() % 1000) * 1000);
    }

    Log() << "RegistrarClient::AddKey() - add key = " << key << " connection = " << connection << endl;

    connections_.insert(KeyConnection::value_type(key, connection));

    if ( clients_.count(connection) == 0 )
    {
        Client* client = new Client("", connection);
        client->SetQueueSize(queue_size_);
        clients_.insert(ConnectionClient::value_type(connection, client));

        Log() << "RegistrarClient::AddKey() - add client = " << client << " connection = " << connection << endl;
    }

    zframe_destroy(&key_frame);
}

string RegistrarClient::ReceiveAnswer(zframe_t* key)
{
    zmq_pollitem_t items[] = { { socket_, 0, ZMQ_POLLIN, 0 } };

    if ( zmq_poll(items, 1, kReadAnswerTimeout) <= 0 )
    {
        Log() << "RegistrarClient::ReceiveAnswer() - error = " << zmq_strerror(zmq_errno()) << " (" << zmq_errno << ")" << endl;
        return "";
    }

    zmsg_t* msg = zmsg_recv(socket_);
    zframe_t* key_frame = zmsg_pop(msg);

    if ( ! zframe_eq(key_frame, key) )
    {
        zframe_destroy(&key_frame);
        zmsg_destroy(&msg);
        return "";
    }

    zframe_t* connection_frame = zmsg_pop(msg);
    char* buffer =  zframe_strdup(connection_frame);
    string connection = buffer;

    free(buffer);
    zframe_destroy(&key_frame);
    zframe_destroy(&connection_frame);
    zmsg_destroy(&msg);

    return connection;

}

void RegistrarClient::SetQueueSize(int size)
{
    queue_size_ = size;
}
