#ifndef DETSVR_CONFIG_H
#define DETSVR_CONFIG_H

#include <string>
#include "detcore/io.h"

namespace detsvr
{
class Config
{
public:
    IInput::Param inParam;
    IOutput::Param outParam;

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