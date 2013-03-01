#ifndef CLIENT_H
#define CLIENT_H

#include <string>

#include "debug.h"
#include "zero_cache.h"

namespace zero_cache
{

class Client : protected Debug
{
public:
    Client(const char* log_file);
    virtual ~Client();

    void WriteData(const std::string& key, const Package package) const;
    Package ReadData(const std::string& key);
    std::string GetKeys();

private:
    int dev_file_;
};

}

#endif
