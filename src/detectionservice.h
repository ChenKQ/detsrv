#ifndef DETSVR_DETECTION_SERVICE_H
#define DETSVR_DETECTION_SERVICE_H

#include "service.h"
#include "detcore/utils.h"
#include "config.h"
#include "detcore/detection.h"
#include "detcore/plugincore.h"
#include <memory>

namespace detsvr
{
class DetectionService final : public Service
{
public:
    DetectionService(Config& cfg ,Logger& logger);
    virtual ~DetectionService() override = default;

    int initialize() override;
    void start() override;
private:
    Config& m_cfg;
    Logger& m_logger;
    PluginFactory factory;
    std::shared_ptr<IDetect> m_pdetector;
};

}

#endif