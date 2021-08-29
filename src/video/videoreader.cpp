#include "videoreader.h"
#include "registry.h"

namespace detsvr
{

using RtspReaderBuilder = Builder<IInput, RtspReader>;
using CSICameraReaderBuilder = Builder<IInput, CSICameraReader>;

template<>
std::map<std::string, std::function<Factory<IInput>::CreateFunc>> 
Factory<IInput>::repository = 
{
    {"rtsp", RtspReaderBuilder::CreateInstance},
    {"csi", CSICameraReaderBuilder::CreateInstance}
};

bool OpenCVReader::read(cv::Mat& outImage)
{
    bool flag = cap.read(outImage);
    return flag;
}

bool RtspReader::open(void* uri)
{

    std::string pipeline = 
            std::string{"rtspsrc location="} + static_cast<char*>(uri) + 
            " latency=0 " + "! queue ! rtph264depay " +
            "! h264parse ! nvv4l2decoder enable-max-performance=1 " +
            // "! rtph264depay ! h264parse ! omxh264dec  disable-dvfs=1 " +
            "! nvvidconv ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
    std::cout << "Using gstreamer pipeline: " << pipeline << "\n";

    cap.open(pipeline, cv::CAP_GSTREAMER);
    if(!cap.isOpened())
    {
        std::cerr << "Failed to open camera." << std::endl;
        return false;
    }

    std::cout << "opened the rtsp stream: " << uri << "\n";
    // play();
    return true;
}

bool CSICameraReader::open(void* uri)
{
    std::string pipeline = std::string{} + 
            "nvarguscamerasrc ! video/x-raw(memory:NVMM)" + 
            // ", width=(int)" + std::to_string(capture_width) + 
            // ", height=(int)" + std::to_string(capture_height) + 
            ", format=(string)NV12" +
            // ", framerate=(fraction)" + std::to_string(framerate) +"/1" +
            " ! nvvidconv " + 
            // "flip-method=" + std::to_string(flip_method) + 
            " ! video/x-raw " + 
            // ", width=(int)" + std::to_string(display_width) + 
            // ", height=(int)" + std::to_string(display_height) + 
            ", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
    std::cout << "Using gstreamer pipeline: " << pipeline << "\n";

    cap.open(pipeline, cv::CAP_GSTREAMER);
    if(!cap.isOpened())
    {
        std::cerr << "Failed to open camera." << std::endl;
        return false;
    }
    std::cout << "opened the rtsp streamed...\n";

    return true;
}

};