#include "server_base.h"

#include <assert.h>

#include "interrupt_signal.h"
#include "functions.h"
#include "connection.h"
#include "socket_list.h"

using namespace std;
using namespace zero_cache;

ServerBase::ServerBase(const char* log_file, Connection connection, const SocketType type) :
    Debug(log_file), socket_(type), connection_(connection)
{
    SocketList::Instance(type);

    socket_.BindIn(connection);
    socket_.SetQueueSize(1000);
}

void ServerBase::Start(long timeout)
{
    AssignSignalHandler();

    while ( ! gIsInterrupt )
        ProcessMessage(timeout);
}

void ServerBase::ProcessMessage(long timeout)
{
    request_.Receive(socket_, timeout);

    connection_.SetHost(request_.GetHost());

    SocketList* out_sockets = SocketList::Instance();
    out_sockets->CreateSocket(connection_, request_.GetId());

    PerformCommand();
}
