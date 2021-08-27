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
    virtual DetectionResult detect(int rows, int cols, int type, void* data, size_t step) = 0;
};

extern "C"
{
    std::shared_ptr<IDetect> createInstance();
}


}

#endif