#ifndef _DETSVR_ISERIALIZE_
#define _DETSVR_ISERIALIZE_

#include "detsvr/detectionresult.h"
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

void to_json(nlohmann::ordered_json& j, const DetectionResult& box);
void from_json(const nlohmann::ordered_json& j, DetectionResult& box);

void to_json(nlohmann::ordered_json& j, const ServiceConfig& box);
void from_json(const nlohmann::ordered_json& j, ServiceConfig& box);

void to_json(nlohmann::ordered_json& j, const PluginConfig& box);
void from_json(const nlohmann::ordered_json& j, PluginConfig& box);

void to_json(nlohmann::ordered_json& j, const Config& box);
void from_json(const nlohmann::ordered_json& j, Config& box);

} // end namespace detsvr

#endif