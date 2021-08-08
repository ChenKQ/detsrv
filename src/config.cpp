#include "config.h"
#include "ISerialize.h"

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

void to_json(nlohmann::ordered_json& j, const ServiceConfig& cfg)
{
    j = nlohmann::ordered_json
    {
        {"listenIP", cfg.listenIP},
        {"port", cfg.port},
        {"baseDir", cfg.baseDir},
        {"NIC", cfg.NIC}
    };
}
void from_json(const nlohmann::ordered_json& j, ServiceConfig& cfg)
{
    j.at("listenIP").get_to(cfg.listenIP);
    j.at("port").get_to(cfg.port);
    j.at("baseDir").get_to(cfg.baseDir);
    j.at("NIC").get_to(cfg.NIC);
}

void to_json(nlohmann::ordered_json& j, const PluginConfig& cfg)
{
    j = nlohmann::ordered_json
    {
        {"filename", cfg.filename},
    };
}
void from_json(const nlohmann::ordered_json& j, PluginConfig& cfg)
{
    j.at("filename").get_to(cfg.filename);
}

void to_json(nlohmann::ordered_json& j, const LicenseConfig& cfg)
{
    j = nlohmann::ordered_json
    {
        {"license", cfg.license},
    };
}
void from_json(const nlohmann::ordered_json& j, LicenseConfig& cfg)
{
    j.at("license").get_to(cfg.license);
}

void to_json(nlohmann::ordered_json& j, const Config& cfg)
{
    j = nlohmann::ordered_json
    {
        {"ServiceConfig", cfg.svrCfg},
        {"PluginConfig", cfg.pluginCfg},
        {"LicenseConfig", cfg.licenseCfg}
    };
}
void from_json(const nlohmann::ordered_json& j, Config& cfg)
{
    j.at("ServiceConfig").get_to(cfg.svrCfg);
    j.at("PluginConfig").get_to(cfg.pluginCfg);
    j.at("LicenseConfig").get_to(cfg.licenseCfg);
}

} // end namespace detsvr