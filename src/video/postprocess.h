#ifndef DET_POSTPROCESS_H
#define DET_POSTPROCESS_H

#include "detsvr/detsvr.h"

namespace detsvr
{

class PaintDetectionResult : public IPostProcess
{
public:
    bool process(cv::Mat& img, DetectionResult& result) override;
    ~PaintDetectionResult() override= default;
}; // PaintDetectionResult

} // namespace detsvr

#endif