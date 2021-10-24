#include "serialization.h"
#include "detcore/io.h"
#include "detection.h"
#include "config.h"
#include <sstream>
#include <string>

namespace detsvr
{
/************************************* 
* ******** detection result **********
*************************************/
void to_json(nlohmann::ordered_json& j, const BBox& box)
{
    std::stringstream strm;
    strm.precision(4);
    strm << box.prob;
    j = nlohmann::ordered_json 
    {
        {"idx", box.idx},
        {"name", box.name},
        {"prob", strm.str()},
        {"minx", box.minx},
        {"maxx", box.maxx},
        {"miny", box.miny},
        {"maxy", box.maxy}
    };
}
void from_json(const nlohmann::ordered_json& j, BBox& box)
{
    j.at("idx").get_to(box.idx);
    j.at("name").get_to(box.name);

    std::string probstr = j.at("prob").get<std::string>();
    double prob = std::stod(probstr);
    box.prob = prob;

    j.at("minx").get_to(box.minx);
    j.at("maxx").get_to(box.maxx);
    j.at("miny").get_to(box.miny);
    j.at("maxy").get_to(box.maxy);
}

void to_json(nlohmann::ordered_json& j, const DetectionResult& res)
{
    j = nlohmann::ordered_json
    {
        {"img_tag", res.img_tag},
        {"img_time", res.img_time},
        {"img_height", res.img_height},
        {"img_width", res.img_width},
        {"pre_time", res.pre_time},
        {"inf_time", res.inf_time},
        {"list", res.list}
    };
}
void from_json(const nlohmann::ordered_json& j, DetectionResult& res)
{
    j.at("img_tag").get_to(res.img_tag);
    j.at("img_time").get_to(res.img_time);
    j.at("img_height").get_to(res.img_height);
    j.at("img_width").get_to(res.img_width);
    j.at("pre_time").get_to(res.pre_time);
    j.at("inf_time").get_to(res.inf_time);
    j.at("list").get_to(res.list);
}

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