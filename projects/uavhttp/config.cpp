#include "config.h"
#include "detcore/serialize.h"
#include "serialization.h"
#include <string>
#include <fstream>
#include <sstream>

namespace detsvr
{

Config& Config::GetInstance()
{
    static Config cfg;
    return cfg;
}

void Config::load(const std::string& configFile)
{
    std::ifstream fstrm(configFile, std::ios::in);
    if(!fstrm.is_open())
    {
        return;
    }
    std::stringstream buffer;
    buffer << fstrm.rdbuf();

    Config& cfg = Config::GetInstance();
    ISerialize<Config>::Deserialize(cfg, buffer.str());
}

void Config::dump(const std::string& configFile)
{
    Config& cfg = Config::GetInstance();
    std::string jsonStr = ISerialize<Config>::Serialize(cfg);

    std::ofstream fstrm(configFile, std::ios::out);
    if(!fstrm.is_open())
    {
        return;
    }
    fstrm.write(jsonStr.c_str(), jsonStr.length());
    fstrm.close();
}

} // end namespace detsvr