#ifndef VIDEO_RTSP_CAPTURE_H
#define VIDEO_RTSP_CAPTURE_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <queue>

namespace detsvr
{

class RtspCapture final
{
    enum PlayStatus
    {
        ERROR = -2,
        STOP = -1,
        PAUSE = 0,
        PLAY = 1
    };
public:
    RtspCapture(int bufferSize = 8);
    ~RtspCapture();

    // copy and assignment are not allowed, so as the move by default
    RtspCapture(const RtspCapture& rhs) = delete;
    RtspCapture& operator=(const RtspCapture& rhs) = delete;

    bool open(const std::string& uri);
    void close();
    void play();
    void pause();
    bool read(cv::Mat& outImage);

    inline bool isOpen() const;

private:
    void run();

private:
    cv::VideoCapture cap;

    std::vector<cv::Mat> imagePool;
    std::queue<cv::Mat> buffer;
    const int bufferSize;
    
    /**
     * play status: -2:error, -1:stop, 0: pause, 1: play,
     **/
    std::atomic<int> playStatus;
    std::thread receiveThread;
    std::mutex poolMutex;
    std::mutex bufferMutex;
    std::mutex pauseMutex;
    std::condition_variable signalPause;

    const int FAILURELIMIT = 100;
};

} // end namespace

#endif