#ifndef DETSVR_DETECTION_SERVICE_H
#define DETSVR_DETECTION_SERVICE_H

#include "detcore/utils.h"
#include "config.h"
#include "detection.h"
#include <memory>

namespace detsvr
{
class DetectionService final
{
public:
    DetectionService(Config& cfg ,::detsvr::Logger& logger);
    ~DetectionService() = default;

    int initialize() ;
    void start() ;
private:
    Config& m_cfg;
    ::detsvr::Logger& m_logger;
    httplib::Server svr;
    // PluginFactory factory;
    std::shared_ptr<IDetect> m_pdetector;
};

}

#endif