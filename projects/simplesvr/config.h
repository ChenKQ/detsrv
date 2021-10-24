#ifndef DETSVR_CONFIG_H
#define DETSVR_CONFIG_H

#include <string>

namespace detsvr
{

typedef struct _ServiceConfig
{
    std::string listenIP {"0.0.0.0"};
    int port {8080};
    std::string baseDir {"./"};
    std::string NIC {"eth0"};
} ServiceConfig;

class Config
{
public:
    ServiceConfig svrCfg;
public:
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    ~Config() = default;

    static Config& GetInstance();

    static void load(const std::string& configFile);
    static void dump(const std::string& configFile);

private:
    Config() = default;
};

} // endnamespace detsvr

#endif