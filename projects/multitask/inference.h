#ifndef _DETSVR_DETECTION_MINIC_
#define _DETSVR_DETECTION_MINIC_

#include "detection.h"
#include "logging.h"
#include "yolov5.hpp"
#include "yololayer.h"
#include "NvInfer.h"
#include "NvInferRuntime.h"
#include "common.hpp"
#include <opencv2/opencv.hpp>
#include <tuple>

namespace detsvr
{

class Inference final : public IDetect 
{
public:
    Inference();
    virtual ~Inference();

    DetectionResult detect(int rows, int cols, int type, void* pdata, size_t step) override;
private:
    void preprocessImg(cv::Mat& img);
    void doInference(float* input, float* det_output, int* seg_output);

private:
    cudaStream_t stream;
    void* buffers[4];
    cv::Mat preprocessedImage;
    cv::Mat resizedImage;
    cv::Mat fittedImage;
    std::unique_ptr<float[]> data {nullptr};
    std::unique_ptr<float[]> det {nullptr};
    std::unique_ptr<int[]> seg {nullptr};
    // std::unique_ptr<int[]> lane {nullptr};

    IRuntime* runtime = nullptr;
    ICudaEngine* engine = nullptr;
    IExecutionContext* context = nullptr;

    const int deviceID = 0;
    const double nmsThresh = 0.4;
    const double confidenceThresh = 0.5;
    
    const std::string engineName = "yolopcut.engine";
    const int batchSize = 1;
    const int inputWidth = Yolo::INPUT_W;
    const int inputHeight = Yolo::INPUT_H;
    const int imageWidthFit = Yolo::IMG_W;
    const int imageHeightFit = Yolo::IMG_H;
    const int maxOutputBBoxCount = Yolo::MAX_OUTPUT_BBOX_COUNT;
    const int classNumber = Yolo::CLASS_NUM;
    const int outputSize = Yolo::MAX_OUTPUT_BBOX_COUNT * sizeof(Yolo::Detection) / sizeof(float) + 1;

    const char* inputBlobName = INPUT_BLOB_NAME;
    const char* outputDetName = OUTPUT_DET_NAME;
    const char* outputSegName = OUTPUT_SEG_NAME;
    // const char* outputLaneName = OUTPUT_LANE_NAME;

    Logger gLogger;
};


} // namespace detsvr

#endif