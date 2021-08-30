#ifndef DETSVR_VIDEO_READER_H
#define DETSVR_VIDEO_READER_H

#include "detsvr/detsvr.h"

#include <map>
#include <queue>
#include <mutex>
#include <vector>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <opencv2/opencv.hpp>


namespace detsvr
{

class OpenCVReader : public IInput
{
public:
    OpenCVReader() = default;
    virtual ~OpenCVReader() = default;
    OpenCVReader(const OpenCVReader& other) = delete; // nocopy
    OpenCVReader& operator= (const OpenCVReader& rhs) = delete; // no assnignment    virtual bool open(const std::string& uri) = 0;
    
    virtual bool open(void* uri) =0;
    void close() override { cap.release(); }
    bool read(cv::Mat& outImage) override { return cap.read(outImage); }
    bool isOpen() const override {return cap.isOpened(); }

protected:
    cv::VideoCapture cap;
}; // OpenCVReader

class RtspReader final: public OpenCVReader
{
public:
    RtspReader() = default;
    ~RtspReader() override { close(); }
    
    bool open(void* uri) override;
}; // RtspReader

// not tested by now
class CSICameraReader final : public OpenCVReader
{
public:
    CSICameraReader() = default;
    ~CSICameraReader() override { close(); }

    bool open(void* uri) override;
}; // CSICameraReader

// to be implemented in the future
class VideoFileReader final : public OpenCVReader {};

class OpenCVWriter : public IOutput
{
public:
    OpenCVWriter() = default;
    ~OpenCVWriter() override = default;

    virtual bool open(void* params) = 0;
    void close() override { writer.release(); }
    bool write(cv::Mat& image) override;
    bool isOpen() const override { return writer.isOpened(); }

protected:
    cv::VideoWriter writer;
};

class RtmpWriter final: public OpenCVWriter
{
public:
    typedef struct _Params
    {
        std::string uri;
        int fps;
        int displayWidth;
        int displayHeight;
        bool isColor;
    } Params;
public:
    RtmpWriter() = default;
    ~RtmpWriter() override { close(); }

    bool open(void* params) override;
};

// to be implementd in the future
class RtspWriter final : public OpenCVWriter {};
class RtspServer final : public OpenCVWriter {};
class FileWriter final : public OpenCVWriter {};
class ScreenWriter final : public OpenCVWriter {};

class PlayManager final
{
public:
    enum Status
    {
        ERROR = -2,
        STOP = -1,
        RUN = 1
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

    int getStatus() const { return playStatus; }

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

class WriteManager final
{
    using ImageResultPair = std::tuple<cv::Mat, std::shared_ptr<DetectionResult>>;
public:
    enum Status
    {
        ERROR = -2,
        STOP = -1,
        RUN = 1
    };

public:
    WriteManager(const std::shared_ptr<IOutput>& output, int bufferSize=8);
    ~WriteManager();
    bool start();
    void stop();
    bool write(cv::Mat& image, DetectionResult& result);

    int getStatus() const { return status; }

private:
    void run();

private:
    std::shared_ptr<IOutput> writer;

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
}; //WriterManager


} // namespace detsvr

#endif