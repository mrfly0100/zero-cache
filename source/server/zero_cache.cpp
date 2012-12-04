#include "reactor.h"

#include <iostream>

#include "program_options.h"

using namespace std;
using namespace zero_cache;

void PrintUsage()
{
    cout << "Usage: zero_cached [options]" << endl;
    cout << "Options:" << endl;
    cout << "\t-c CONNECTION\t\tSet connection string" << endl;
    cout << "\t-l FILE\t\t\tSet log file name" << endl;
    cout << "\t-s SIZE\t\t\tSet size of the input queue" << endl;
    cout << "\t-h\t\t\tPrint option help" << endl;
}

int main(int argc, char *argv[])
{
    ProgramOptions options(argv, argv+argc);

    if ( options.IsOptionExist("-h") )
    {
        PrintUsage();
        return 1;
    }

    string log_file = "";
    if ( options.IsOptionExist("-l") )
        log_file = options.GetOption("-l");

    string connection = "tcp://*:5570";
    if ( options.IsOptionExist("-c") )
        connection = options.GetOption("-c");

    Reactor reactor(log_file, connection);

    if ( options.IsOptionExist("-s") )
        reactor.SetQueueSize(atoi(options.GetOption("-s")));

    reactor.Start();

    return 0;
}
