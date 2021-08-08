#ifndef _DETSVR_CONFIG_
#define _DETSVR_CONFIG_

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

typedef struct _PluginConfig
{
    std::string filename;
} PluginConfig;

typedef struct _LicenseConfig
{
    std::string license;
} LicenseConfig;

class Config
{
public:
    ServiceConfig svrCfg;
    PluginConfig pluginCfg;
    LicenseConfig licenseCfg;

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