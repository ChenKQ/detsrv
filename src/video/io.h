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
#include <string>
#include <functional>
#include <condition_variable>
#include <opencv2/opencv.hpp>


namespace detsvr
{

/**
 *@brief OpenCV读取视频的抽象类，open方法为纯虚函数 
 * 
 */
class OpenCVReader : public IInput
{
public:
    OpenCVReader() = default;
    virtual ~OpenCVReader() = default;
    OpenCVReader(const OpenCVReader& other) = delete; // nocopy
    OpenCVReader& operator= (const OpenCVReader& rhs) = delete; // no assnignment    virtual bool open(const std::string& uri) = 0;
    
    virtual bool open(void* uri) =0;
    void close() override { cap.release(); }
    bool read(cv::Mat& outImage) override;
    bool isOpen() const override {return cap.isOpened(); }

protected:
    cv::VideoCapture cap;
}; // OpenCVReader

/**
 * @brief 基于OpenCV和gstreamer读取rtsp视频流
 * */
class RtspReader final: public OpenCVReader
{
public:
    RtspReader() = default;
    ~RtspReader() override { close(); }
    
    /**
     * @brief open: 打开
     * 
     * @param uri : 实际参数类型为char*
     * @return true 
     * @return false 
     */
    bool open(void* uri) override;
}; // RtspReader


/**
 * @brief 基于OpenCV和gstreamer读取CSI摄像头
 * 尚未测试
 * 
 */
class CSICameraReader final : public OpenCVReader
{
public:
    CSICameraReader() = default;
    ~CSICameraReader() override { close(); }

    /**
     * @brief open: 打开摄像头
     * 
     * @param uri ：不需要传入参数，调用时传入nullptr即可
     * @return true 
     * @return false 
     */
    bool open(void* uri) override;
}; // CSICameraReader

// to be implemented in the future
class VideoFileReader final : public OpenCVReader {};

/**
 * @brief OpenCV输出视频的抽象类，open方法为纯虚函数
 * 
 */
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

/**
 * @brief 基于OpenCV和gstreamer输出rtmp视频流
 * 
 */
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

    /**
     * @brief open 打开
     * 
     * @param params void*类型为Params* 
     * @return true 
     * @return false 
     */
    bool open(void* params) override;
};

// to be implementd in the future
class RtspWriter final : public OpenCVWriter {};
class RtspServer final : public OpenCVWriter {};
class FileWriter final : public OpenCVWriter {};

/**
 * @brief ScreenWriter 将视频输出到屏幕
 * 
 */
class ScreenWriter final : public IOutput
{
public:
    using Params = std::string;
public:
    ScreenWriter() = default;;
    ~ScreenWriter() override { close(); }

    /**
     * @brief open 打开
     * 
     * @param params void*类型为Params*类型
     * @return true 打开成功
     * @return false 打开失败
     */
    bool open(void* params) override;
    void close() override;
    bool write(cv::Mat& image) override;
    bool isOpen() const override { return openFlag; }
private:
    bool openFlag = false;
    std::string windowName;
};

/**
 * @brief 播放管理器，创建该实例前先创建IInput实现类的实例
 * 该类采用多线程异步模型，start方法调用后将开启线程不断调用IInput实例的read方法
 * 该类的read方法由其他线程调用
 * 
 */
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

/**
 * @brief 输出管理器，创建该实例前请先创建IOutput实例
 * 该类采用多线程异步模型，start方法调用后将开启线程不断调用IOutput实例的write方法
 * 该类的write方法由其他线程调用
 */
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