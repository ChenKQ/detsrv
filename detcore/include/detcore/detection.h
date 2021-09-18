#ifndef DETSVR_IDETECT_H
#define DETSVR_IDETECT_H

#include <memory>
#include <vector>

namespace detsvr
{
/**
 * @brief Bounding Box of an object 
 */
typedef struct _BBox
{
    int idx;    // the ith object in the detection result
    std::string name; // class id
    double prob; // the confidence
    int minx; 
    int maxx; 
    int miny; 
    int maxy;
} BBox;

/**
 * @brief Detection Result
 */ 
typedef struct _DetectionResult
{
    std::string img_tag; // source
    std::string img_time; // "YYYY-MM-DD hh:mm:ss"
    int img_height; // pixel
    int img_width; // pixel
    int pre_time; // ms
    int inf_time; // ms

    std::vector<BBox> list; // detected objects
} DetectionResult;

class IDetect
{
public:
    virtual ~IDetect() = default;

    virtual DetectionResult detect(const char* data, size_t length) = 0;
    virtual DetectionResult detect(int rows, int cols, int type, void* data, size_t step) = 0;
};

}  // end namespace detsvr

extern "C"
{
    std::shared_ptr<detsvr::IDetect> createInstance();
}


#endif