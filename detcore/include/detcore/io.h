#ifndef DETSVR_VIDEO_IO_H
#define DETSVR_VIDEO_IO_H

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
 * @brief 输入接口类
 * 
 * */
class IInput
{
public:
    typedef struct _Param
    {
        std::string InType;
        std::string Uri;
    } Param;
public:
    virtual bool open(const Param& params) = 0;
    virtual void close() = 0;
    virtual bool read(cv::Mat& outImage) = 0;
    virtual bool isOpen() const = 0;

    virtual ~IInput() = default;
}; // IInput

/**
 * @brief 输出接口类
 * 
 */
class IOutput
{
public:
    typedef struct _Param
    {
        std::string OutType;
        std::string Protocol;
        std::string Ip;
        std::string Port;
        std::string Index;
        int Width;
        int Height;
        int FPS;
    } Param;
public:
    virtual bool open(const Param& params) = 0;
    virtual void close() = 0;
    virtual bool write(cv::Mat& image) = 0;
    virtual bool isOpen() const = 0;

    virtual ~IOutput() = default;
}; // IOutput

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
    
    virtual bool open(const IInput::Param& params) =0;
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
     * @param params : 
     * @return true 
     * @return false 
     */
    bool open(const IInput::Param& params) override;
}; // RtspReader

class RtmpReader final : public OpenCVReader
{
public:
    RtmpReader() = default;
    ~RtmpReader() override { close();  }

    /**
     * @brief open: 打开
     * 
     * @param params：
     * @return true 
     * @return false 
     */
    bool open(const IInput::Param& params) override;
};  // RtmpReader

/**
 * @brief 基于OpenCV和gstreamer读取CSI摄像头
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
     * @param uri ：
     * @return true 
     * @return false 
     */
    bool open(const IInput::Param& params) override;
}; // CSICameraReader

/**
 * @brief 基于OpenCV和gstreamer读取CSI摄像头
 * 
 */
class USBCameraReader final : public OpenCVReader
{
public:
    USBCameraReader() = default;
    ~USBCameraReader() override { close(); }

    /**
     * @brief open: 打开摄像头
     * 
     * @param uri ：
     * @return true 
     * @return false 
     */
    bool open(const IInput::Param& params) override;
}; // USBCameraReader

class Mp4FileReader final : public OpenCVReader
{
public:
    Mp4FileReader() = default;
    ~Mp4FileReader() override { close();  }

    /**
     * @brief open: 打开
     * 
     * @param params：
     * @return true 
     * @return false 
     */
    bool open(const IInput::Param& params) override;
};  // Mp4FileReader

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

    virtual bool open(const IOutput::Param& params) = 0;
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
    RtmpWriter() = default;
    ~RtmpWriter() override { close(); }

    /**
     * @brief open 打开
     * 
     * @param params 
     * @return true 
     * @return false 
     */
    bool open(const IOutput::Param& params) override;
};

// to be implementd in the future
class RtspWriter final : public OpenCVWriter
{
public:
    RtspWriter() = default;
    ~RtspWriter() override { close(); }

    /**
     * @brief open 打开
     * 
     * @param params
     * @return true 
     * @return false 
     */
    bool open(const IOutput::Param& params) override;
};

class RtspServer final : public IOutput
{
public:
    RtspServer();
    ~RtspServer() override;

    bool open(const IOutput::Param& params) override;
    void close() override;
    bool write(cv::Mat& image) override;
    bool isOpen() const override;

private:
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

    Context context;
    std::thread t_server;
    // std::atomic<bool> isRunning;

    const int failureLimit = 50;
    // std::mutex imgMutex;
    // int count = 0;
    // cv::Mat frameImage;
public:
    void run(const IOutput::Param& params);
private:
    static GstRTSPMediaFactory* createRTSPMediaFactory(Context* ctx);

    static void needData(GstElement* appsrc, guint unused, Context* ctx);
    static void mediaConfigure(GstRTSPMediaFactory* factory, 
                               GstRTSPMedia* media, gpointer userData);
    static void clientConnected(GstRTSPServer* server, GstRTSPClient* client, gpointer user_data);
    static void clientClosed(GstRTSPClient* client, gpointer user_data);
    // static gboolean disconnect(GstRTSPClient* client);
    static gboolean removeSession(GstRTSPClient* client);
    // static void tearDown(GstRTSPClient* client, GstRTSPContext* ctx, gpointer user_data);
    void start(const IOutput::Param& params);
};
class FileWriter final : public OpenCVWriter {};

/**
 * @brief ScreenWriter 将视频输出到屏幕
 * 
 */
class ScreenWriter final : public IOutput
{
public:
    ScreenWriter() = default;;
    ~ScreenWriter() override { close(); }

    /**
     * @brief open 打开
     * 
     * @param params
     * @return true 打开成功
     * @return false 打开失败
     */
    bool open(const IOutput::Param& params) override;
    void close() override;
    bool write(cv::Mat& image) override;
    bool isOpen() const override { return openFlag; }
private:
    bool openFlag = false;
    std::string windowName;
    int displayWidth;
    int displayHeight;
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
    PlayManager(int bufferSize = 8);
    ~PlayManager();

    // copy and assignment are not allowed, so as the move by default
    PlayManager(const PlayManager& rhs) = delete;
    PlayManager& operator=(const PlayManager& rhs) = delete;

    bool start(const std::shared_ptr<IInput>& input);
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
template<typename RES>
class WriteManager final
{
    using ImageResultPair = std::tuple<cv::Mat, std::shared_ptr<RES>>;
    using Action = bool (cv::Mat& img, RES& result);
public:
    enum Status
    {
        ERROR = -2,
        STOP = -1,
        RUN = 1
    };

public:
    WriteManager(int bufferSize=8);
    ~WriteManager();
    bool start(std::function<Action> act);
    void stop();
    bool write(cv::Mat& image, RES& result);

    int getStatus() const { return status; }

private:
    void run();

private:
    // std::shared_ptr<IOutput> writer;
    std::function<Action> action;

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

template<typename RES>
WriteManager<RES>::WriteManager(int bufSize): bufferSize(bufSize),
                                              action(nullptr)
                                                    
{
    // assert(output!=nullptr);
    status = Status::STOP;
    for(int i=0; i<bufSize; ++i)
    {
        RES* p = new RES{};
        imageResultPool.push_back({cv::Mat{}, std::shared_ptr<RES>(p)});
    }
    // writer = output;
}

template<typename RES>
WriteManager<RES>::~WriteManager()
{
    stop();
}

template<typename RES>
bool WriteManager<RES>::start(std::function<Action> act)
{
    action = act;
    // if(!writer->isOpen())
    // {
    //     std::cout << "the output source is not open ...\n";
    //     return false;
    // }

    if(status == Status::STOP)
    {
        status = Status::RUN;
        pushThread = std::thread([obj=this]
        {
            obj->run();

        });
        pushThread.detach();
    }
    return true;
}

template<typename RES>
void WriteManager<RES>::stop()
{
    status = Status::STOP;
    signalNew.notify_all();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    newMutex.lock();
    newMutex.unlock();

    action = nullptr;
}

template<typename RES>
bool WriteManager<RES>::write(cv::Mat& image, RES& result)
{
    ImageResultPair imgResPair;
    std::unique_lock<std::mutex> poolLock(poolMutex);
    if(!imageResultPool.empty())
    {
        // fetch from image-result pool
        imgResPair = imageResultPool.back();
        imageResultPool.pop_back();
        poolLock.unlock();
        // std::cout << "from image-result pool...\n";
    }
    else 
    {
        // fetch from buffer
        // poolLock.unlock();
        std::unique_lock<std::mutex> bufferLock(bufferMutex);
        imgResPair = buffer.front();
        buffer.pop();
        bufferLock.unlock();
        std::cout << "lost one frame...\n";
        // std::cout << "from writer buffer...\n";
    }
    cv::Mat& bufferImg = std::get<0>(imgResPair);
    std::shared_ptr<RES> pRes = std::get<1>(imgResPair);
    std::swap(bufferImg, image);
    std::swap(result, *pRes);

    std::unique_lock<std::mutex> bufferLock(bufferMutex);
    buffer.push(imgResPair);
    if(buffer.size()==1)
    {
        signalNew.notify_all();
    }
    return true;
}

template<typename RES>
void WriteManager<RES>::run()
{
    // status = WriteStatus::PLAY;
    int failureCount = 0;
    ImageResultPair imgResPair;

    std::unique_lock<std::mutex> sigLock(newMutex);
    while(true)
    {
        // if(!writer->isOpen())
        // {
        //     std::cout << "Exit the writing thread since the writer is not open...\n";
        //     status = Status::STOP;
        //     return;
        // }

        if(action==nullptr)
        {
            std::cout << "no action ...\n";
            status = Status::STOP;
            return;
        }

        if(status==Status::STOP)
        {
            // writer->close();
            std::cout << "stop writting...\n";
            return;
        }

        while(buffer.empty())
        {
            signalNew.wait(sigLock);

            if(status==Status::STOP)
            {
                // writer->close();
                std::cout << "stop writting when blocked...\n";
                return;
            }
        }
        std::unique_lock<std::mutex> bufferLock(bufferMutex);
        imgResPair = buffer.front();
        buffer.pop();
        bufferLock.unlock();

        cv::Mat& img = std::get<0>(imgResPair);
        std::shared_ptr<RES> result = std::get<1>(imgResPair);
        // for(const auto& box : result->list)
        // {
        //     cv::Point2d tl(box.minx, box.miny);
        //     cv::Point2d br(box.maxx, box.maxy);
        //     cv::rectangle(img, tl, br, cv::Scalar(0x27, 0xC1, 0x36), 2);
        //     cv::putText(img, box.name, cv::Point(box.minx, box.maxy - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
        // }
        if(action==nullptr || !action(img, *result))
        {
            std::unique_lock<std::mutex> poolLock(poolMutex);
            imageResultPool.push_back(imgResPair);
            poolLock.unlock();
            
            ++failureCount;
            if(failureCount >= FAILURELIMIT)
            {
                status = Status::ERROR;
                std::cout << "failure number of output exceeds the limit...\n";
                return ;
            }
        }
        else 
        {
            std::unique_lock<std::mutex> poolLock(poolMutex);
            imageResultPool.push_back(imgResPair);
            poolLock.unlock();
            failureCount = 0;
        }
    }
}

} // namespace detsvr

#endif