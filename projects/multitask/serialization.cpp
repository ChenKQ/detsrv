#include "serialization.h"
#include "detcore/io.h"
#include "detection.h"
#include "config.h"
#include <sstream>
#include <string>

namespace detsvr
{
/************************************* 
* ******** input and output params **********
*************************************/
void to_json(nlohmann::ordered_json& j, const IInput::Param& p)
{
    j = nlohmann::ordered_json
    {
        {"InType", p.InType},
        {"Uri", p.Uri}
    };
}
void from_json(const nlohmann::ordered_json& j, IInput::Param& p)
{
    j.at("InType").get_to(p.InType);
    j.at("Uri").get_to(p.Uri);
}

void to_json(nlohmann::ordered_json& j, const IOutput::Param& p)
{
    j = nlohmann::ordered_json
    {
        {"OutType", p.OutType},
        {"Protocol", p.Protocol},
        {"Ip", p.Ip},
        {"Port", p.Port},
        {"Index", p.Index},
        {"Width", p.Width},
        {"Height", p.Height},
        {"FPS", p.FPS}
    };
}
void from_json(const nlohmann::ordered_json& j, IOutput::Param& p)
{
    j.at("OutType").get_to(p.OutType);
    j.at("Protocol").get_to(p.Protocol);
    j.at("Ip").get_to(p.Ip);
    j.at("Port").get_to(p.Port);
    j.at("Index").get_to(p.Index);
    j.at("Width").get_to(p.Width);
    j.at("Height").get_to(p.Height);
    j.at("FPS").get_to(p.FPS);
}

/************************************* 
* ******** config file **********
*************************************/
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
        // {"ServiceConfig", cfg.svrCfg},
        {"PluginConfig", cfg.pluginCfg},
        {"LicenseConfig", cfg.licenseCfg},
        {"Input", cfg.inParam},
        {"Output", cfg.outParam}
    };
}
void from_json(const nlohmann::ordered_json& j, Config& cfg)
{
    // j.at("ServiceConfig").get_to(cfg.svrCfg);
    j.at("PluginConfig").get_to(cfg.pluginCfg);
    j.at("LicenseConfig").get_to(cfg.licenseCfg);
    j.at("Input").get_to(cfg.inParam);
    j.at("Output").get_to(cfg.outParam);
}
} // end namespace detsvr