#ifndef _DETSVR_DETECTION_SERVICE_
#define _DETSVR_DETECTION_SERVICE_

#include "service.h"
#include "utils.h"
#include "config.h"
#include "detsvr/IDetect.h"
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
    std::shared_ptr<IDetect> m_pdetector;
};

}

#endif