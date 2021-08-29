#ifndef VIDEO_RTMP_WRITER_H
#define VIDEO_RTMP_WRITER_H

#include "detsvr/detsvr.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <tuple>
#include <atomic>
#include <thread>
#include <memory>
#include <condition_variable>
#include <functional>

namespace detsvr
{

// class ImageResultPair;
using ImageResultPair = std::tuple<cv::Mat, std::shared_ptr<DetectionResult>>;

class RtmpWriter final
{
    enum WriteStatus
    {
        ERROR = -2,
        STOP = -1,
        PLAY = 1
    };
public:
    using FuseFunc = void (cv::Mat& image, const DetectionResult& result, 
                           cv::VideoWriter& w);
public:
    RtmpWriter(int bufferSize);
    ~RtmpWriter();
    bool open(const std::string& uri, int fps, int displayWidth, int displayHeight, 
              bool isColor, const std::function<FuseFunc>& func);
    void close();
    bool write(cv::Mat& image, DetectionResult& result);

private:
    void run();

private:
    cv::VideoWriter writer;
    std::function<FuseFunc> consume; 

    std::vector<ImageResultPair> imageResultPool;
    std::queue<ImageResultPair> buffer;
    const int bufferSize;

    std::atomic<int> status;
    std::thread pushThread;
    std::mutex poolMutex;
    std::mutex bufferMutex;
    std::mutex newMutex;
    std::condition_variable signalNew;

    const int FAILURELIMIT = 100;

}; // RtmpWriter

} // end namespace detsvr

#endif