#ifndef DETSVR_SERIALIZATION_H
#define DETSVR_SERIALIZATION_H

#include "detcore/io.h"
#include "config.h"
#include "nlohmann/json.hpp"
#include "detection.h"
#include <string>

namespace detsvr
{
// input out params
void to_json(nlohmann::ordered_json& j, const IInput::Param& p);
void from_json(const nlohmann::ordered_json& j, IInput::Param& p);

void to_json(nlohmann::ordered_json& j, const IOutput::Param& p);
void from_json(const nlohmann::ordered_json& j, IOutput::Param& p);

// configures
void to_json(nlohmann::ordered_json& j, const PluginConfig& cfg);
void from_json(const nlohmann::ordered_json& j, PluginConfig& cfg);

void to_json(nlohmann::ordered_json& j, const LicenseConfig& cfg);
void from_json(const nlohmann::ordered_json& j, LicenseConfig& cfg);

void to_json(nlohmann::ordered_json& j, const Config& cfg);
void from_json(const nlohmann::ordered_json& j, Config& cfg);
} // end namespace detsvr

#endif