#ifndef _DETSVR_DETECTION_MINIC_
#define _DETSVR_DETECTION_MINIC_

#include "detection.h"
#include "logging.h"
#include "yololayer.h"
#include "NvInfer.h"
#include "NvInferRuntime.h"
#include "common.hpp"
#include <opencv2/opencv.hpp>
#include <tuple>

namespace detsvr
{

class UAVShip final : public IDetect 
{
public:
    UAVShip();
    virtual ~UAVShip();

    DetectionResult detect(const char* data, size_t length) override; 
    // DetectionResult detect(int rows, int cols, int type, void* pdata, size_t step) override;
private:
    void preprocessImg(cv::Mat& img);
    void doInference(float* input, float* output);

private:
    cudaStream_t stream;
    void* buffers[2];
    cv::Mat decodedImage;
    cv::Mat resizedImage;
    cv::Mat preprocessedImage;
    std::unique_ptr<float[]> data {nullptr};
    std::unique_ptr<float[]> prob {nullptr};

    IRuntime* runtime = nullptr;
    ICudaEngine* engine = nullptr;
    IExecutionContext* context = nullptr;

    const int deviceID = 0;
    const double nmsThresh = 0.4;
    const double confidenceThresh = 0.5;
    
    const std::string engineName = "uavship.engine";
    const int batchSize = 1;
    const int inputWidth = Yolo::INPUT_W;
    const int inputHeight = Yolo::INPUT_H;
    const int maxOutputBBoxCount = Yolo::MAX_OUTPUT_BBOX_COUNT;
    const int classNumber = Yolo::CLASS_NUM;
    const int outputSize = Yolo::MAX_OUTPUT_BBOX_COUNT * sizeof(Yolo::Detection) / sizeof(float) + 1;

    const char* inputBlobName = "data";
    const char* outputBlobName = "prob";

    Logger gLogger;
};


} // namespace detsvr

#endif