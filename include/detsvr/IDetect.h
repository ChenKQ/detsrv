#ifndef _DETSVR_IDETECT_
#define _DETSVR_IDETECT_

#include "detectionresult.h"
#include <memory>

namespace detsvr
{

class IDetect
{
public:
    virtual ~IDetect() = default;

    virtual DetectionResult detect(const char* data, size_t length) = 0;
};

extern "C"
{
    std::shared_ptr<IDetect> createInstance();
}


}

#endif