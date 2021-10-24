#ifndef _DETSVR_DETECTION_MINIC_
#define _DETSVR_DETECTION_MINIC_

#include "detection.h"

namespace detsvr
{

class DetectionMinic final : public IDetect 
{
public:
    // DetectionMinic() = default;
    virtual ~DetectionMinic() override = default;

    DetectionResult detect(const char* data, size_t length) override; 
};


} // namespace detsvr

#endif