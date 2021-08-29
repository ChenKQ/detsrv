#include "io.h"
#include "detsvr/detsvr.h"

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

bool OpenCVWriter::write(cv::Mat& image)
{
    if(!isOpen())
    {
        return false;
    }
    writer.write(image);

}

bool RtmpWriter::open(void* params)
{
    Params* p = reinterpret_cast<Params*>(params);
    std::string writePipeline = 
            std::string{"appsrc ! queue ! videoconvert"} +
            "! nvvidconv ! omxh264enc " +
            "! flvmux streamable=true ! rtmpsink location=" + p->uri;
    int FOURCC = cv::VideoWriter::fourcc('H', '2', '6', '4');
    writer.open( writePipeline, cv::CAP_GSTREAMER, FOURCC,
                static_cast<double>(p->fps), 
                cv::Size(p->displayWidth, p->displayHeight), 
                p->isColor);
    if(!writer.isOpened())
    {
        std::cout <<"cannot open video writer: "<< p->uri <<  std::endl;
        return -1;
    }
    std::cout << "opened the video writer: " << p->uri << std::endl;
    return true;
}

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

RtmpWriter::RtmpWriter(const std::shared_ptr<IOutput>& output, int bufSize)
{
    assert(output!=nullptr);
    status = Status::STOP;
    for(int i=0; i<bufSize; ++i)
    {
        DetectionResult* p = new DetectionResult{};
        imageResultPool.push_back({cv::Mat{}, std::shared_ptr<DetectionResult>(p)});
    }
    writer = output;
}

RtmpWriter::~RtmpWriter()
{
    close();
}

bool RtmpWriter::start(const std::shared_ptr<IPostProcess>& processor)
{
    if(!writer.isOpen())
    {
        std::cout << "the output source is not open ...\n";
        return false;
    }
    postProcessor = processor;

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

void RtmpWriter::stop()
{
    status = Status::STOP;
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

            if(status==Status::STOP)
            {
                writer.release();
                std::cout << "stop writting when blocked...\n";
                return;
            }
        }

        imgResPair = buffer.front();
        buffer.pop();
        // bufferLock.unlock();

        cv::Mat& img = std::get<0>(imgResPair);
        std::shared_ptr<DetectionResult> result = std::get<1>(imgResPair);
        if(postProcessor!=nullptr)
        {            
            postProcessor->process(img, *result, writer);
        }
        for(const detsvr::BBox& box : result.list)
        {
            cv::Point2d tl(box.minx, box.miny);
            cv::Point2d br(box.maxx, box.maxy);
            cv::rectangle(image, tl, br, cv::Scalar(0x27, 0xC1, 0x36), 2);
            cv::putText(image, box.name, cv::Point(box.minx, box.maxy - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
        }
        writer.write(img);
        
        // std::unique_lock<std::mutex> poolLock(poolMutex);
        imageResultPool.push_back(imgResPair);
        // poolLock.unlock();
    }
}

};