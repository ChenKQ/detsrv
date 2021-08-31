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

using RtmpWriterBuilder = Builder<IOutput, RtmpWriter>;
using ScreenWriterBuilder = Builder<IOutput, ScreenWriter>;

template<>
std::map<std::string, std::function<Factory<IOutput>::CreateFunc>>
Factory<IOutput>::repository = 
{
    {"rtmp", RtmpWriterBuilder::CreateInstance},
    {"screen", ScreenWriterBuilder::CreateInstance}
};


bool OpenCVReader::read(cv::Mat& outImage)
{
    if(!isOpen())
        return false;
    return cap.read(outImage);
}

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
    return true;
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

bool ScreenWriter::open(void* params)
{
    if(openFlag)
        return true;

    std::string* p = (Params*)params;
    windowName = *p;
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    
    openFlag = true;
    return openFlag;
}

void ScreenWriter::close()
{
    if(!openFlag)
        return;
    openFlag = false;
    cv::destroyAllWindows();
}

bool ScreenWriter::write(cv::Mat& image)
{
    if(!isOpen())
        return false;
    cv::imshow(windowName,image);
    int keycode = cv::waitKey(10) & 0xff ; 
    if (keycode == 27)
    {
        openFlag = false;
        std::cout << "get key value: " << keycode << "\n";
    }
        
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
        playStatus = Status::RUN;
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
        if(!cap->isOpen())
        {
            std::cout << "Exit the fetching thread since the capture is not open...\n";
            playStatus = Status::STOP;
            return;
        }

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
                std::cout << "failure number of input exceeds the limit...\n";
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

WriteManager::WriteManager(const std::shared_ptr<IOutput>& output, int bufSize): 
                                                    bufferSize(bufSize)
                                                    
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

WriteManager::~WriteManager()
{
    stop();
}

bool WriteManager::start()
{
    if(!writer->isOpen())
    {
        std::cout << "the output source is not open ...\n";
        return false;
    }

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

void WriteManager::stop()
{
    status = Status::STOP;
    signalNew.notify_all();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    newMutex.lock();
    newMutex.unlock();
}

bool WriteManager::write(cv::Mat& image, DetectionResult& result)
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

void WriteManager::run()
{
    // status = WriteStatus::PLAY;
    int failureCount = 0;
    ImageResultPair imgResPair;

    std::unique_lock<std::mutex> sigLock(newMutex);
    while(true)
    {
        if(!writer->isOpen())
        {
            std::cout << "Exit the writing thread since the writer is not open...\n";
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
        // std::unique_lock<std::mutex> bufferLock(bufferMutex);
        imgResPair = buffer.front();
        buffer.pop();
        // bufferLock.unlock();

        cv::Mat& img = std::get<0>(imgResPair);
        std::shared_ptr<DetectionResult> result = std::get<1>(imgResPair);
        for(const detsvr::BBox& box : result->list)
        {
            cv::Point2d tl(box.minx, box.miny);
            cv::Point2d br(box.maxx, box.maxy);
            cv::rectangle(img, tl, br, cv::Scalar(0x27, 0xC1, 0x36), 2);
            cv::putText(img, box.name, cv::Point(box.minx, box.maxy - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
        }
        if(!writer->write(img))
        {
            // std::unique_lock<std::mutex> poolLock(poolMutex);
            imageResultPool.push_back(imgResPair);
            // poolLock.unlock();
            
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
            // std::unique_lock<std::mutex> poolLock(poolMutex);
            imageResultPool.push_back(imgResPair);
            // poolLock.unlock();
            failureCount = 0;
        }


    }
}

};