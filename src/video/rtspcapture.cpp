#include "rtspcapture.h"

namespace detsvr
{
RtspCapture::RtspCapture(int bufSize) : bufferSize(bufSize),
                                        imagePool(bufSize)
{
    playStatus = PlayStatus::STOP; 
    for(int i=0; i<bufSize; ++i)
    {
        imagePool.push_back(cv::Mat{});
    }
}

RtspCapture::~RtspCapture()
{
    close();
}

bool RtspCapture::open(const std::string& uri)
{
    std::string pipeline = 
            std::string{"rtspsrc location="} + uri + " latency=0 " + "! queue " +
            "! rtph264depay ! h264parse ! nvv4l2decoder enable-max-performance=1 " +
            "! nvvidconv ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
    std::cout << "Using gstreamer pipeline: " << pipeline << "\n";

    cap.open(pipeline, cv::CAP_GSTREAMER);
    if(!cap.isOpened())
    {
        std::cerr << "Failed to open camera." << std::endl;
        return false;
    }

    std::cout << "opened the rtsp streamed...\n";
    // play();
    return true;
}

void RtspCapture::close()
{
    playStatus = PlayStatus::STOP;
    signalPause.notify_all();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    pauseMutex.lock();
    pauseMutex.unlock();
}

void RtspCapture::pause()
{
    playStatus = PlayStatus::PAUSE;
}

void RtspCapture::play()
{
    if(playStatus==PlayStatus::STOP)
    {
        playStatus = PlayStatus::PLAY;
        receiveThread = std::thread([obj=this]
        {
            // obj->playStatus = RtspCapture::PlayStatus::PLAY;
            obj->run();
        });
        receiveThread.detach();
        return;
    }

    if(playStatus!=PlayStatus::PAUSE)
    {
        return;
    }

    playStatus = PlayStatus::PLAY;
    signalPause.notify_all();
}

bool RtspCapture::read(cv::Mat& outImage)
{
    // std::unique_lock<std::mutex> bufferLock(bufferMutex);
    if(buffer.empty())
    {
        return false;
    }
    cv::Mat img = buffer.front();
    buffer.pop();
    // bufferLock.unlock();

    std::swap(img, outImage);

    // std::unique_lock<std::mutex> poolLock(poolMutex);
    imagePool.push_back(img);
    return true;
}

void RtspCapture::run()
{
    // playStatus = PlayStatus::PLAY;
    int failureCount = 0;
    cv::Mat img;
    std::unique_lock<std::mutex> pauseLock(pauseMutex);

    while(true)
    {
        if(playStatus==PlayStatus::STOP)
        {
            cap.release();
            std::cout << "stop fetching video ...\n";
            return;
        }

        while(playStatus==PlayStatus::PAUSE)
        {
            std::cout << "pause...\n";
            signalPause.wait(pauseLock);

            if(playStatus==PlayStatus::STOP)
            {
                cap.release();
                std::cout << "stop when paused ...\n";
                return;
            }

        }
        // fetch one from image pool or from buffer
        // std::unique_lock<std::mutex> poolLock(poolMutex);
        if(!imagePool.empty())
        {
            // fetch from image pool
            img = imagePool.back();
            imagePool.pop_back();
            // poolLock.unlock();
            std::cout << "from capture image pool...\n";
        }
        else 
        {
            // fetch from buffer
            // poolLock.unlock();
            // std::unique_lock<std::mutex> bufferLock(bufferMutex);
            img = buffer.front();
            buffer.pop();
            // bufferLock.unlock();
            std::cout << "from capture buffer...\n";
        }
        
        // read image
        if(!cap.read(img))
        {
            // poolLock.lock();
            imagePool.push_back(img);
            // poolLock.unlock();

            ++ failureCount;
            std::cout << "fail to fetch image, count: " << failureCount;
            if(failureCount >= FAILURELIMIT)
            {
                playStatus = PlayStatus::ERROR;
                cap.release();
                return;
            }
        }
        else 
        {
            failureCount = 0;

            // put into the buffer
            // std::unique_lock<std::mutex> bufferLock(bufferMutex);
            buffer.push(img);
            // bufferLock.unlock();
        }

        

    }

}

} // namespace detsvr