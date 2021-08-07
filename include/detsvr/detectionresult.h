#ifndef _DETSVR_DETECTION_RESULT_
#define _DETSVR_DETECTION_RESULT_

#include <string>
#include <vector>

namespace detsvr
{

typedef struct _BBox
{
    int idx; 
    std::string name;
    double prob; 
    int minx;
    int maxx;
    int miny;
    int maxy;
} BBox;

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

    
} // namespace detsvr


#endif