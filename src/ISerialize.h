#ifndef _DETSVR_ISERIALIZE_
#define _DETSVR_ISERIALIZE_

#include "detsvr/detsvr.h"
#include "config.h"
#include "nlohmann/json.hpp"
#include <string>

namespace detsvr
{

template<typename T>
struct ISerialize
{
    static std::string Serialize(const T& obj);
    static void Deserialize(T& obj, const std::string& s);
};

template <typename T>
std::string ISerialize<T>::Serialize(const T& obj)
{
    nlohmann::ordered_json j = obj;
    return j.dump(4);
}

template<typename T>
void ISerialize<T>::Deserialize(T& obj, const std::string& s)
{
    nlohmann::json j = nlohmann::json::parse(s);
    j.get_to(obj);
}

void to_json(nlohmann::ordered_json& j, const BBox& box);
void from_json(const nlohmann::ordered_json& j, BBox& box);

void to_json(nlohmann::ordered_json& j, const DetectionResult& result);
void from_json(const nlohmann::ordered_json& j, DetectionResult& result);

void to_json(nlohmann::ordered_json& j, const ServiceConfig& cfg);
void from_json(const nlohmann::ordered_json& j, ServiceConfig& cfg);

void to_json(nlohmann::ordered_json& j, const PluginConfig& cfg);
void from_json(const nlohmann::ordered_json& j, PluginConfig& cfg);

void to_json(nlohmann::ordered_json& j, const LicenseConfig& cfg);
void from_json(const nlohmann::ordered_json& j, LicenseConfig& cfg);

void to_json(nlohmann::ordered_json& j, const Config& cfg);
void from_json(const nlohmann::ordered_json& j, Config& cfg);

} // end namespace detsvr

#endif