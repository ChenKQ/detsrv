#include "rtmpwriter.h"
#include "detsvr/detsvr.h"
#include <opencv2/opencv.hpp>

namespace detsvr
{
RtmpWriter::RtmpWriter(int bufSize) : bufferSize(bufSize),
                                      imageResultPool(bufSize)
{
    status = WriteStatus::STOP;
    for(int i=0; i<bufSize; ++i)
    {
        DetectionResult* p = new DetectionResult{};
        imageResultPool.push_back({cv::Mat{}, std::shared_ptr<DetectionResult>(p)});
    }
}

RtmpWriter::~RtmpWriter()
{
    close();
}

bool RtmpWriter::open(const std::string& uri, int fps, 
                     int displayWidth, int displayHeight, 
                     bool isColor, const std::function<FuseFunc>& func)
{
    consume = func;
    std::string writePipeline = std::string{"appsrc ! queue ! videoconvert"} +
                        "! nvvidconv ! omxh264enc " +
                        "! flvmux streamable=true ! rtmpsink location=" + uri;

    int FOURCC = cv::VideoWriter::fourcc('H', '2', '6', '4');
    writer.open( writePipeline, cv::CAP_GSTREAMER, FOURCC,
                static_cast<double>(fps), 
                cv::Size(displayWidth, displayHeight), 
                true);
    if(!writer.isOpened())
    {
        std::cout <<"cannot open video writer: "<< uri <<  std::endl;
        return -1;
    }
    std::cout << "opened the video writer: " << uri << std::endl;

    status = WriteStatus::PLAY;
    pushThread = std::thread([obj=this]
    {
        obj->run();

    });
    pushThread.detach();
    return true;
}

void RtmpWriter::close()
{
    status = WriteStatus::STOP;
    signalNew.notify_all();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    newMutex.lock();
    newMutex.unlock();
}

bool RtmpWriter::write(cv::Mat& image, DetectionResult& result)
{
    ImageResultPair imgResPair;
    // std::unique_lock<std::mutex> poolLock(poolMutex);
    if(!imageResultPool.empty())
    {
        // fetch from image-result pool
        imgResPair = imageResultPool.back();
        imageResultPool.pop_back();
        // poolLock.unlock();
        // std::cout << "from image-result pool...\n";
    }
    else 
    {
        // fetch from buffer
        // poolLock.unlock();
        // std::unique_lock<std::mutex> bufferLock(bufferMutex);
        imgResPair = buffer.front();
        buffer.pop();
        // bufferLock.unlock();
        // std::cout << "from writer buffer...\n";
    }
    cv::Mat& bufferImg = std::get<0>(imgResPair);
    std::shared_ptr<DetectionResult> pRes = std::get<1>(imgResPair);
    std::swap(bufferImg, image);
    std::swap(result, *pRes);

    // std::unique_lock<std::mutex> bufferLock(bufferMutex);
    buffer.push(imgResPair);
    if(buffer.size()==1)
    {
        signalNew.notify_all();
    }
    return true;
}

void RtmpWriter::run()
{
    // status = WriteStatus::PLAY;
    ImageResultPair imgResPair;

    std::unique_lock<std::mutex> sigLock(newMutex);
    while(true)
    {
        if(status==WriteStatus::STOP)
        {
            writer.release();
            std::cout << "stop writting...\n";
            return;
        }

        // std::unique_lock<std::mutex> bufferLock(bufferMutex);
        while(buffer.empty())
        {
            signalNew.wait(sigLock);

            if(status==WriteStatus::STOP)
            {
                writer.release();
                std::cout << "stop writting when blocked...\n";
                return;
            }
        }

        imgResPair = buffer.front();
        buffer.pop();
        // bufferLock.unlock();

        if(consume!=nullptr)
        {
            cv::Mat& img = std::get<0>(imgResPair);
            std::shared_ptr<DetectionResult> result = std::get<1>(imgResPair);
            consume(img, *result, writer);
        }

        // std::unique_lock<std::mutex> poolLock(poolMutex);
        imageResultPool.push_back(imgResPair);
        // poolLock.unlock();
    }
}
    
} // namespace detsvr
