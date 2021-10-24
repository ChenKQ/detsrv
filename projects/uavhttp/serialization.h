#ifndef DETSVR_SERIALIZATION_H
#define DETSVR_SERIALIZATION_H

#include "config.h"
#include "nlohmann/json.hpp"
#include "detection.h"
#include <string>

namespace detsvr
{
// detection results
void to_json(nlohmann::ordered_json& j, const BBox& box);
void from_json(const nlohmann::ordered_json& j, BBox& box);

void to_json(nlohmann::ordered_json& j, const DetectionResult& result);
void from_json(const nlohmann::ordered_json& j, DetectionResult& result);


// configures
void to_json(nlohmann::ordered_json& j, const ServiceConfig& cfg);
void from_json(const nlohmann::ordered_json& j, ServiceConfig& cfg);


void to_json(nlohmann::ordered_json& j, const LicenseConfig& cfg);
void from_json(const nlohmann::ordered_json& j, LicenseConfig& cfg);

void to_json(nlohmann::ordered_json& j, const Config& cfg);
void from_json(const nlohmann::ordered_json& j, Config& cfg);
} // end namespace detsvr

#endif