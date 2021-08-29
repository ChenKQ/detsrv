#include "detsvr/detsvr.h"

namespace detsvr
{
bool PaintDetectionResult::process(cv::Mat& img, DetectionResult& result) 
{
    for(const detsvr::BBox& box : result.list)
    {
        cv::Point2d tl(box.minx, box.miny);
        cv::Point2d br(box.maxx, box.maxy);
        cv::rectangle(image, tl, br, cv::Scalar(0x27, 0xC1, 0x36), 2);
        cv::putText(image, box.name, cv::Point(box.minx, box.maxy - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
    }
}
} // namespace detsvr