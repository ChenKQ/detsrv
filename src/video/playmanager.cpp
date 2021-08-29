#include "playmanager.h"
#include "detsvr/detsvr.h"

#include <cassert>

namespace detsvr
{
PlayManager::PlayManager(const std::shared_ptr<IInput>& input, int bufSize) : 
                                        bufferSize(bufSize),
                                        imagePool(bufSize)
{
    assert(input!=nullptr);
    playStatus = Status::STOP; 
    for(int i=0; i<bufSize; ++i)
    {
        imagePool.push_back(cv::Mat{});
    }
    cap = input;
}

PlayManager::~PlayManager()
{
    stop();
}

bool PlayManager::start()
{
    if(!cap->isOpen())
    {
        std::cout << "the input source is not open...\n";
        return false;
    }
    if(playStatus == Status::STOP)
    {
        playStatus = Status::PLAY;
        receiveThread = std::thread([obj=this]
        {
            obj->run();
        });
        receiveThread.detach();
    }
    return true;
}

void PlayManager::stop()
{
    playStatus = Status::STOP;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    runMutex.lock();
    runMutex.unlock();
}

bool PlayManager::read(cv::Mat& outImage)
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

void PlayManager::run()
{
    // playStatus = PlayStatus::PLAY;
    int failureCount = 0;
    cv::Mat img;
    std::unique_lock<std::mutex> runLock(runMutex);

    while(true)
    {
        if(playStatus==Status::STOP)
        {
            // cap->close();
            std::cout << "stop fetching video ...\n";
            return;
        }

        // fetch one from image pool or from buffer
        // std::unique_lock<std::mutex> poolLock(poolMutex);
        if(!imagePool.empty())
        {
            // fetch from image pool
            img = imagePool.back();
            imagePool.pop_back();
            // poolLock.unlock();
            // std::cout << "from capture image pool...\n";
        }
        else 
        {
            // fetch from buffer
            // poolLock.unlock();
            // std::unique_lock<std::mutex> bufferLock(bufferMutex);
            img = buffer.front();
            buffer.pop();
            std::cout << "drop one frame in capture...\n";
            // bufferLock.unlock();
            // std::cout << "from capture buffer...\n";
        }
        
        // read image
        if(!cap->read(img))
        {
            // poolLock.lock();
            imagePool.push_back(img);
            // poolLock.unlock();

            ++ failureCount;
            std::cout << "fail to fetch image, count: " << failureCount << "\n";
            if(failureCount >= FAILURELIMIT)
            {
                playStatus = Status::ERROR;
                std::cout << "failure number exceeds the limit...\n";
                // cap->close();
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