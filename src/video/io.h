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
#include <glib.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

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
    
    virtual bool open(void* params) =0;
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
    using Params = std::string;
public:
    RtspReader() = default;
    ~RtspReader() override { close(); }
    
    /**
     * @brief open: 打开
     * 
     * @param params : 实际参数类型为std::string*
     * @return true 
     * @return false 
     */
    bool open(void* params) override;
}; // RtspReader

class RtmpReader final : public OpenCVReader
{
public:
    using Params = std::string;
public:
    RtmpReader() = default;
    ~RtmpReader() override { close();  }

    /**
     * @brief open: 打开
     * 
     * @param params： 实际参数类型为std::string*
     * @return true 
     * @return false 
     */
    bool open(void* params) override;
};


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
    bool open(void* params) override;
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
class RtspWriter final : public OpenCVWriter
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
    RtspWriter() = default;
    ~RtspWriter() override { close(); }

    /**
     * @brief open 打开
     * 
     * @param params void*类型为Params* 
     * @return true 
     * @return false 
     */
    bool open(void* params) override;

};

class RtspServer final : public IOutput
{
public:
    typedef struct _Params
    {
        std::string outIndex;
        std::string outPort;
        int outWidth;
        int outHeight;
        int outFPS;
    } Params;

    typedef struct _Context
    {
        // gboolean white;
        GstClockTime timestamp;
        int outWidth;
        int outHeight;
        int outFPS;
        std::string outIndex;
        std::string outPort;
        int count ;

        std::mutex imgMutex;
        cv::Mat frameImage;

        GMainLoop *loop;
    } Context;

public:
    RtspServer();
    ~RtspServer() override;

    bool open(void* params) override;
    void close() override;
    bool write(cv::Mat& image) override;
    bool isOpen() const override;

protected:
    Context context;
    std::thread t_server;
    std::atomic<bool> isRunning;

    const int failureLimit = 50;
    // std::mutex imgMutex;
    // int count = 0;
    // cv::Mat frameImage;
public:
    void run(const Params& param);
private:
    static GstRTSPMediaFactory* createRTSPMediaFactory(Context* ctx);

    static void needData(GstElement* appsrc, guint unused, Context* ctx);
    static void mediaConfigure(GstRTSPMediaFactory* factory, 
                               GstRTSPMedia* media, gpointer userData);
    static void clientConnected(GstRTSPServer* server, GstRTSPClient* client, gpointer user_data);
    static void clientClosed(GstRTSPClient* client, gpointer user_data);
    // static gboolean disconnect(GstRTSPClient* client);
    // static gboolean removeSession(GstRTSPClient* client);
    // static void tearDown(GstRTSPClient* client, GstRTSPContext* ctx, gpointer user_data);
    // static GstRTSPFilterResult sessionFilterFunc(GstRTSPClient * client,
    //                                GstRTSPSession * sess,
    //                                gpointer user_data);
    void start(const Params& param);
};
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