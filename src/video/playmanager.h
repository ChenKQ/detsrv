#ifndef DETSVR_PLAY_MANAGER_H
#define DETSVR_PLAY_MANAGER_H

#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <vector>
#include <queue>
#include "videoreader.h"


namespace detsvr
{

class PlayManager final
{
public:
    enum Status
    {
        ERROR = -2,
        STOP = -1,
        PLAY = 1
    };
public:
    PlayManager(const std::shared_ptr<IInput>& input, int bufferSize = 8);
    ~PlayManager();

    // copy and assignment are not allowed, so as the move by default
    PlayManager(const PlayManager& rhs) = delete;
    PlayManager& operator=(const PlayManager& rhs) = delete;

    bool start();
    void stop();
    bool read(cv::Mat& outImage);

    PlayManager::Status getStatus() const;

private:
    void run();

private:
    std::shared_ptr<IInput> cap;

    std::vector<cv::Mat> imagePool;
    std::queue<cv::Mat> buffer;
    const int bufferSize;
    
    std::atomic<int> playStatus;
    std::thread receiveThread;
    std::mutex poolMutex;
    std::mutex bufferMutex;
    std::mutex runMutex;
    // std::condition_variable signalPause;

    const int FAILURELIMIT = 100;
};

} // namespace detsvr

#endif