#include "client.h"

#include "connection.h"
#include "functions.h"

using namespace std;
using namespace zero_cache;

static const long kReadAnswerTimeout = 1000;

Client::Client(const char* log_file, Connection connection, SocketType type) : Debug(log_file), socket_(type)
{
    srand(time(NULL));

    Log() << "Client::Client() - connect " << connection.GetString() << endl;
    socket_.ConnectOut(connection);

    port_t id = GenerateId(this);
    connection.SetPort(id);

    SetHost(connection.GetHost());

    if ( connection.GetProtocol() == kTcpProtocol )
        connection.SetHost("*:");

    Log() << "Client::Client() - bind " << connection.GetString() << endl;
    socket_.BindIn(connection);
    socket_.SetQueueSize(10);

     zmq_msg_init(&host_msg_);
     zmq_msg_init(&id_msg_);
     zmq_msg_init(&command_msg_);
     zmq_msg_init(&key_msg_);
     zmq_msg_init(&data_msg_);
}

Client::~Client()
{
    zmq_msg_close(&data_msg_);
    zmq_msg_close(&key_msg_);
    zmq_msg_close(&command_msg_);
    zmq_msg_close(&id_msg_);
    zmq_msg_close(&host_msg_);
}

void Client::WriteData(string& key, void* data, size_t size)
{
    Log() << "Client::WriteData() - key = " << key << " data_size = " << size << endl;

    Command command = kWrite;

    zmq_msg_init_data(&command_msg_, &command, sizeof(Command), NULL, NULL);
    zmq_msg_init_data(&key_msg_, (void*)key.c_str(), key.size(), NULL, NULL);
    zmq_msg_init_data(&data_msg_, data, size, NULL, NULL);

    socket_.SendMsg(command_msg_, ZMQ_SNDMORE);
    socket_.SendMsg(key_msg_, ZMQ_SNDMORE);
    socket_.SendMsg(id_msg_, ZMQ_SNDMORE);
    socket_.SendMsg(host_msg_, ZMQ_SNDMORE);
    socket_.SendMsg(data_msg_, 0);
}

void* Client::ReadData(string& key)
{
    Log() << "Client::ReadData() - key = " << key << endl;

    void* result = NULL;

    while ( result == NULL )
    {
        SendReadRequest(key);
        result = ReceiveReadAnswer();

        if (result == NULL )
            usleep(rand() % 1000);
    }

    return result;
}

void Client::SendReadRequest(string& key)
{
    Command command = kRead;

    zmq_msg_init_data(&command_msg_, &command, sizeof(Command), NULL, NULL);
    zmq_msg_init_data(&key_msg_, (void*)key.c_str(), key.size(), NULL, NULL);

    socket_.SendMsg(command_msg_, ZMQ_SNDMORE);
    socket_.SendMsg(key_msg_, ZMQ_SNDMORE);
    socket_.SendMsg(id_msg_, ZMQ_SNDMORE);
    socket_.SendMsg(host_msg_, 0);
}

void* Client::ReceiveReadAnswer()
{
    if ( ! socket_.ReceiveMsg(kReadAnswerTimeout) )
        return NULL;

    zmq_msg_t key_frame = socket_.PopMsg();

    if ( ! IsMsgEqual(key_frame, key_msg_) )
    {
        zmq_msg_close(&key_frame);
        return NULL;
    }

    zmq_msg_t msg = socket_.PopMsg();
    void* data = malloc(zmq_msg_size(&msg));
    memcpy(data, zmq_msg_data(&msg), zmq_msg_size(&msg));
    zmq_msg_close(&msg);

    return data;
}

void Client::SetHost(string host)
{
    zmq_msg_init_data(&host_msg_, (void*)host.c_str(), host.size(), NULL, NULL);
}

void Client::SetQueueSize(int size)
{
    socket_.SetQueueSize(size);
}
