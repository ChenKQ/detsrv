#include "uavship.h"
#include "detection.h"
#include "cuda_utils.h"
#include <fstream>
#include <iostream>
#include <cassert>
#include <memory>
#include <opencv2/imgcodecs.hpp>
#include <algorithm>

namespace detsvr
{

UAVShip::UAVShip()
{
    cudaSetDevice(deviceID);

    std::ifstream file(engineName, std::ios::binary);
    if (!file.good()) {
        std::cerr << "read " << engineName << " error!" << std::endl;
        throw std::runtime_error(std::string{"cannot load engine file: "} + engineName);
    }

    char* trtModelStream = nullptr;
    size_t size = 0;
    file.seekg(0, file.end);
    size = file.tellg();
    file.seekg(0, file.beg);
    trtModelStream = new char[size];
    assert(trtModelStream);
    file.read(trtModelStream, size);
    file.close();

    // prepare input data ---------------------------
    float *pData = new float[batchSize * 3 * inputHeight * inputHeight];
    data.reset(pData);
    float *pProb = new float[batchSize * outputSize];
    prob.reset(pProb);

    runtime = createInferRuntime(gLogger);
    assert(runtime != nullptr);
    engine = runtime->deserializeCudaEngine(trtModelStream, size);
    assert(engine != nullptr);
    context = engine->createExecutionContext();
    assert(context != nullptr);
    delete[] trtModelStream;
    assert(engine->getNbBindings() == 2);
    // In order to bind the buffers, we need to know the names of the input and output tensors.
    // Note that indices are guaranteed to be less than IEngine::getNbBindings()

    const int inputIndex = engine->getBindingIndex(inputBlobName);
    const int outputIndex = engine->getBindingIndex(outputBlobName);
    assert(inputIndex == 0);
    assert(outputIndex == 1);

    CUDA_CHECK(cudaMalloc(&buffers[inputIndex], batchSize * 3 * inputHeight * inputWidth * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&buffers[outputIndex], batchSize * outputSize * sizeof(float)));

    CUDA_CHECK(cudaStreamCreate(&stream));
}

UAVShip::~UAVShip()
{
    // Release stream and buffers
    cudaStreamDestroy(stream);
    const int inputIndex = engine->getBindingIndex(inputBlobName);
    const int outputIndex = engine->getBindingIndex(outputBlobName);
    CUDA_CHECK(cudaFree(buffers[inputIndex]));
    CUDA_CHECK(cudaFree(buffers[outputIndex]));
    // Destroy the engine
    context->destroy();
    engine->destroy();
    runtime->destroy();
}

void UAVShip::doInference(float* input, float* output) 
{
    // DMA input batch data to device, infer on the batch asynchronously, and DMA output back to host
    CUDA_CHECK(cudaMemcpyAsync(buffers[0], input, batchSize * 3 * inputHeight * inputWidth * sizeof(float), cudaMemcpyHostToDevice, stream));
    context->enqueue(batchSize, buffers, stream, nullptr);
    CUDA_CHECK(cudaMemcpyAsync(output, buffers[1], batchSize * outputSize * sizeof(float), cudaMemcpyDeviceToHost, stream));
    cudaStreamSynchronize(stream);
}

DetectionResult UAVShip::detect(const char* rawData, size_t length)
{
    std::cout << "entry...\n";
    auto start = std::chrono::high_resolution_clock::now();
    const unsigned char* pData = (const unsigned char*)rawData;
    std::vector<unsigned char> imgVec(pData, pData+ length);
    cv::Mat img = cv::imdecode(imgVec, cv::ImreadModes::IMREAD_UNCHANGED);
    if (img.empty()) return {};
    preprocessImg(img); // letterbox BGR to RGB
    auto end = std::chrono::high_resolution_clock::now();
    int pretime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Run inference
    start = std::chrono::high_resolution_clock::now();
    doInference(data.get(), prob.get());
    // end = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<Yolo::Detection>> batch_res(1);
    for (int b = 0; b < batchSize; b++) {
        auto& res = batch_res[b];
        nms(res, &prob[b * outputSize], confidenceThresh, nmsThresh);
    }
    end = std::chrono::high_resolution_clock::now();
    int inftime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    
    DetectionResult result;
    result.img_tag = "";
    result.img_time = "";
    result.img_height = img.rows;
    result.img_width = img.cols;
    result.pre_time = pretime;
    result.inf_time = inftime;
    result.list.clear();
    int count = 0;
    for (int b = 0; b < batchSize; b++) {
        auto& res = batch_res[b];
        for (size_t j = 0; j < res.size(); j++) {
            // std::cout << "box: " << res[j].bbox[0] << ":" << res[j].bbox[1] << ":"
            //           << res[j].bbox[2] << ":" << res[j].bbox[3] << "\n";
            cv::Rect r = get_rect(img, res[j].bbox);
            std::string name = std::to_string(static_cast<int>(res[j].class_id));
            double prob = res[j].conf;
            int minx = std::max(r.x, 0);
            int maxx = std::min(r.x + r.width, img.cols);
            if(minx>=maxx)
            {
                continue;
            }
            int miny = std::max(r.y, 0);
            int maxy = std::min(r.y + r.height, img.rows);
            if(miny>=maxy)
            {
                continue;
            }
            ++count;
            // std::cout << "height is " << (maxy-miny) << "\n";
            BBox box {count, name, prob, minx, maxx, miny, maxy};
            result.list.push_back(box);
            // cv::rectangle(img, r, cv::Scalar(0x27, 0xC1, 0x36), 2);
            // cv::putText(img, std::to_string((int)res[j].class_id), cv::Point(r.x, r.y - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
        }
        // cv::imwrite("saved.png", img);
    }
    
    return std::move(result);
}

DetectionResult UAVShip::detect(int rows, int cols, int type, void* pdata, size_t step)
{
    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat img(rows, cols, type, pdata, step);
    if (img.empty()) return {};
    preprocessImg(img); // letterbox BGR to RGB
    auto end = std::chrono::high_resolution_clock::now();
    int pretime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Run inference
    start = std::chrono::high_resolution_clock::now();
    doInference(data.get(), prob.get());
    // end = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<Yolo::Detection>> batch_res(1);
    for (int b = 0; b < batchSize; b++) {
        auto& res = batch_res[b];
        nms(res, &prob[b * outputSize], confidenceThresh, nmsThresh);
    }
    end = std::chrono::high_resolution_clock::now();
    int inftime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    
    DetectionResult result;
    result.img_tag = "";
    result.img_time = "";
    result.img_height = img.rows;
    result.img_width = img.cols;
    result.pre_time = pretime;
    result.inf_time = inftime;
    result.list.clear();
    int count = 0;
    for (int b = 0; b < batchSize; b++) {
        auto& res = batch_res[b];
        for (size_t j = 0; j < res.size(); j++) {
            // std::cout << "box: " << res[j].bbox[0] << ":" << res[j].bbox[1] << ":"
            //           << res[j].bbox[2] << ":" << res[j].bbox[3] << "\n";
            cv::Rect r = get_rect(img, res[j].bbox);
            std::string name = std::to_string(static_cast<int>(res[j].class_id));
            double prob = res[j].conf;
            int minx = std::max(r.x, 0);
            int maxx = std::min(r.x + r.width, img.cols);
            if(minx>=maxx)
            {
                continue;
            }
            int miny = std::max(r.y, 0);
            int maxy = std::min(r.y + r.height, img.rows);
            if(miny>=maxy)
            {
                continue;
            }
            ++count;
            // std::cout << "height is " << (maxy-miny) << "\n";
            BBox box {count, name, prob, minx, maxx, miny, maxy};
            result.list.push_back(box);
            // cv::rectangle(img, r, cv::Scalar(0x27, 0xC1, 0x36), 2);
            // cv::putText(img, std::to_string((int)res[j].class_id), cv::Point(r.x, r.y - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
        }
        // cv::imwrite("saved.png", img);
    }
    
    return std::move(result);
}

void UAVShip::preprocessImg(cv::Mat& img)
{
    int w, h, x, y;
    float r_w = inputWidth / (img.cols*1.0);
    float r_h = inputHeight / (img.rows*1.0);
    if (r_h > r_w) {
        w = inputWidth;
        h = r_w * img.rows;
        x = 0;
        y = (inputHeight - h) / 2;
    } else {
        w = r_h * img.cols;
        h = inputHeight;
        x = (inputWidth - w) / 2;
        y = 0;
    }

    if((resizedImage.rows!=h) || (resizedImage.cols!=w))
    {
        resizedImage = cv::Mat(h, w, CV_8UC3);
    }
    
    if(preprocessedImage.empty())
    {
        preprocessedImage = cv::Mat(inputHeight, inputWidth, CV_8UC3, cv::Scalar(128, 128, 128));
    }

    // auto start = std::chrono::system_clock::now();
    cv::resize(img, resizedImage, resizedImage.size(), 0, 0, cv::INTER_LINEAR);
    resizedImage.copyTo(preprocessedImage(cv::Rect(x, y, resizedImage.cols, resizedImage.rows)));
    // auto end = std::chrono::system_clock::now();
    // int pretime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    // std::cout << "preprocess time: " << pretime << "us" << std::endl;
    int i = 0;
    for (int row = 0; row < inputHeight; ++row) {
        uchar* uc_pixel = preprocessedImage.data + row * preprocessedImage.step;
        for (int col = 0; col < inputWidth; ++col) {
            data[i] = (float)uc_pixel[2] / 255.0;
            data[i + inputHeight * inputWidth] = (float)uc_pixel[1] / 255.0;
            data[i + 2 * inputHeight * inputWidth] = (float)uc_pixel[0] / 255.0;
            uc_pixel += 3;
            ++i;
        }
    }
    // return;
}

} // namespace detsvr

void *createInstance()
{
    detsvr::IDetect* ptr = new detsvr::UAVShip();
    return ptr;
}

