#include "detcore/io.h"
#include "detcore/factory.h"

namespace detsvr
{

using RtspReaderBuilder = Builder<IInput, RtspReader>;
using RtmpReaderBuilder = Builder<IInput, RtmpReader>;
using CSICameraReaderBuilder = Builder<IInput, CSICameraReader>;
using USBCameraReaderBuilder = Builder<IInput, USBCameraReader>;
using Mp4FileReaderBuilder = Builder<IInput, Mp4FileReader>;

template<>
std::map<std::string, std::function<Factory<IInput>::CreateFunc>> 
Factory<IInput>::repository = 
{
    {"rtsp", RtspReaderBuilder::CreateInstance},
    {"rtmp", RtmpReaderBuilder::CreateInstance},
    {"csi", CSICameraReaderBuilder::CreateInstance},
    {"usb", USBCameraReaderBuilder::CreateInstance},
    {"mp4", Mp4FileReaderBuilder::CreateInstance}
};

using RtspWriterBuilder = Builder<IOutput, RtspWriter>;
using RtspServerBuilder = Builder<IOutput, RtspServer>;
using RtmpWriterBuilder = Builder<IOutput, RtmpWriter>;
using ScreenWriterBuilder = Builder<IOutput, ScreenWriter>;


template<>
std::map<std::string, std::function<Factory<IOutput>::CreateFunc>>
Factory<IOutput>::repository = 
{
    {"rtsp", RtspWriterBuilder::CreateInstance},
    {"rtspserver", RtspServerBuilder::CreateInstance},
    {"rtmp", RtmpWriterBuilder::CreateInstance},
    {"screen", ScreenWriterBuilder::CreateInstance}
};


bool OpenCVReader::read(cv::Mat& outImage)
{
    if(!isOpen())
        return false;
    return cap.read(outImage);
}

bool RtspReader::open(const IInput::Param& params)
{
    std::string uri = params.Uri;
    std::string pipeline = 
            std::string{"rtspsrc latency=0 protocols=tcp location="} + uri + 
            + " !  rtph264depay " +
            // + " ! rtph264depay "
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

    std::cout << "opened the rtsp input stream: " << uri << "\n";
    // play();
    return true;
}

bool CSICameraReader::open(const IInput::Param& params)
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
    std::cout << "opened the csi camera stream...\n";

    return true;
}

bool USBCameraReader::open(const IInput::Param& params)
{
    std::string pipeline = std::string{} + 
            "v4l2src device=/dev/video1" + 
            " ! image/jpeg, width=1920,height=1080,framerate=30/1,format=MJPG" + 
            " ! jpegdec" + 
            // " ! video/x-raw" + 
            // ",width=(int)" + std::to_string(1280) + 
            // ",height=(int)" + std::to_string(720) + 
            // ",format=(string)YUYV" +
            // ",framerate=(fraction)" + std::to_string(20) +"/1" +
            " ! nvvidconv " + 
            // "flip-method=" + std::to_string(flip_method) + 
            " ! video/x-raw(memory:NVMM) " + 
            // ", width=(int)" + std::to_string(display_width) + 
            // ", height=(int)" + std::to_string(display_height) + 
            ", format=(string)NV12 " + 
            " ! nvvidconv" +
            " ! video/x-raw, format=(string)BGRx" +
            " ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
    std::cout << "Using gstreamer pipeline: " << pipeline << "\n";

    cap.open(pipeline, cv::CAP_GSTREAMER);
    if(!cap.isOpened())
    {
        std::cerr << "Failed to open camera." << std::endl;
        return false;
    }
    std::cout << "opened the usb camera stream...\n";

    return true;
}

bool RtmpReader::open(const IInput::Param& params)
{
    std::string uri = params.Uri;
    std::string pipeline = std::string{} + 
            "rtmpsrc location=\"" + uri + " live=1\"" 
            " ! flvdemux ! h264parse" +
            " ! nvv4l2decoder enable-max-performance=1 " +
            "! nvvidconv ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
    std::cout << "Using gstreamer pipeline: " << pipeline << "\n";

    cap.open(pipeline, cv::CAP_GSTREAMER);
    if(!cap.isOpened())
    {
        std::cerr << "Failed to open camera." << std::endl;
        return false;
    }

    std::cout << "opened the rtmp stream: " << uri << "\n";
    // play();
    return true;
}

bool Mp4FileReader::open(const IInput::Param& params)
{
    std::string uri = params.Uri;
    // std::string pipeline = std::string{} + 
    //         "filesrc location=\"" + uri + "\"" +
    //         " ! h264parse" +
    //         " ! nvv4l2decoder enable-max-performance=1 " +
    //         "! nvvidconv ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
    // std::cout << "Using gstreamer pipeline: " << pipeline << "\n";

    // cap.open(uri, cv::CAP_GSTREAMER);
    cap.open(uri);
    if(!cap.isOpened())
    {
        std::cerr << "Failed to open camera." << std::endl;
        return false;
    }

    std::cout << "opened the mp4 file stream: " << uri << "\n";
    // play();
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

bool RtspWriter::open(const IOutput::Param& params)
{
    std::string uri = params.Protocol + "://" + params.Ip 
                      + ":" + params.Port + params.Index;
                        
    std::string writePipeline = 
            std::string{"appsrc is-live=true ! videoconvert"} +
            // "! nvvidconv ! omxh264enc ! h264parse " + 
            " ! nvvidconv ! nvv4l2h264enc preset-level=UltraFastPreset maxperf-enable=1 ! h264parse" +
            " ! rtspclientsink protocols=tcp latency=0 location=" + uri;
    int FOURCC = cv::VideoWriter::fourcc('H', '2', '6', '4');
    writer.open( writePipeline, cv::CAP_GSTREAMER, FOURCC,
                static_cast<double>(params.FPS), 
                cv::Size(params.Width, params.Height), 
                true);
    if(!writer.isOpened())
    {
        std::cout <<"cannot open video writer: "<< uri <<  std::endl;
        return -1;
    }
    std::cout << "opened the rtsp video writer: " << uri << std::endl;
    return true;
}

bool RtmpWriter::open(const IOutput::Param& params)
{
    std::string uri = params.Protocol + "://" + params.Ip 
                      + ":" + params.Port + params.Index;
    std::string writePipeline = 
            std::string{"appsrc is-live=true ! videoconvert"} +
            // "! nvvidconv ! omxh264enc " +
            " ! nvvidconv ! nvv4l2h264enc preset-level=UltraFastPreset maxperf-enable=1 ! h264parse " +
            "! flvmux streamable=true ! rtmpsink location=\"" + uri + " live=1\"";
    int FOURCC = cv::VideoWriter::fourcc('H', '2', '6', '4');
    writer.open( writePipeline, cv::CAP_GSTREAMER, FOURCC,
                static_cast<double>(params.FPS), 
                cv::Size(params.Width, params.Height), 
                true);
    if(!writer.isOpened())
    {
        std::cout <<"cannot open video writer: "<< uri <<  std::endl;
        return -1;
    }
    std::cout << "opened the rtmp video writer: " << uri << std::endl;
    return true;
}

RtspServer::RtspServer()
{
    context.loop = nullptr;
    // isRunning = false;
}

RtspServer::~RtspServer()
{
    close();
}

bool RtspServer::open(const IOutput::Param& params)
{
    if(isOpen())
        return false;

    start(params);
    
    int count = 0;
    while(!isOpen())
    {
        std::cout << "waiting for the rtsp server startup...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        ++count;
        if(count > failureLimit)
        {
            break;
        }
            
    }
    return isOpen();
}

void RtspServer::close()
{
    if(!isOpen())
        return;
    // isRunning = false;
    // g_main_loop_quit(context.loop);
}

bool RtspServer::isOpen() const 
{
    if(context.loop == nullptr)
    {
        return false;
    }
    return g_main_loop_is_running(context.loop);
    // return isRunning;
}

bool RtspServer::write(cv::Mat& image)
{
    std::unique_lock<std::mutex> locker(context.imgMutex);
    // if(image.empty())
    // {
    //     image = cv::Mat::zeros(context.outWidth, context.outHeight, CV_8UC3);
    // }
    std::swap(image, context.frameImage);
    if(!isOpen())
        return false;
    return true;
}

void RtspServer::needData(GstElement* appsrc, guint unused, Context* ctx)
{
    GstBuffer *buffer;
    guint buffersize;
    GstFlowReturn ret;
    GstMapInfo info;

    ++ctx->count;

    std::unique_lock<std::mutex> lock(ctx->imgMutex);
    cv::resize(ctx->frameImage, ctx->frameImage, cv::Size{ctx->outWidth, ctx->outHeight},
               0,0,cv::INTER_AREA);
    buffersize = ctx->frameImage.cols * ctx->frameImage.rows * ctx->frameImage.channels();
    buffer = gst_buffer_new_and_alloc(buffersize);
    unsigned char* imageData = ctx->frameImage.data;
    if(gst_buffer_map(buffer, &info, (GstMapFlags)GST_MAP_WRITE))
    {
        memcpy(info.data, imageData, buffersize);
        gst_buffer_unmap(buffer, &info);
    }
    else 
    {
        std::cout << "cannot map frame image data into GstMapInfo\n";
    }
    lock.unlock();

    // ctx->white = !ctx->white;
    GST_BUFFER_PTS (buffer) = ctx->timestamp;
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, int(ctx->outFPS));
    ctx->timestamp += GST_BUFFER_DURATION (buffer);

    g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
    if (ret != GST_FLOW_OK) 
    {
        std::cout << "fail to emit push-buffer signal\n";
        g_main_loop_quit (ctx->loop);
    }
    gst_buffer_unref (buffer);
}

void RtspServer::mediaConfigure(GstRTSPMediaFactory* factory, 
                               GstRTSPMedia* media, gpointer userData)
{
    std::cout << "configure ...\n";
    GstElement *element, *appsrc;
    Context* ctx = (Context*)(userData);
    // Params *p = (Params*)userData;

    element = gst_rtsp_media_get_element(media);

    appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");

    // g_object_set (G_OBJECT (appsrc), "is-live" , TRUE ,  NULL);
    g_object_set(G_OBJECT(appsrc), 
                  "stream-type", 0,
                  "format", GST_FORMAT_TIME, NULL);
    g_object_set (G_OBJECT (appsrc), "caps",
      gst_caps_new_simple ("video/x-raw",
          "format", G_TYPE_STRING, "BGR",
          "width", G_TYPE_INT, int(ctx->outWidth),
          "height", G_TYPE_INT, int(ctx->outHeight),
          "framerate", GST_TYPE_FRACTION, int(ctx->outFPS), 1,
          "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1, NULL), NULL);
    ctx->timestamp = 0;

    // gst_rtsp_media_set_latency(media, 10);
    // std::cout << "latency: " << gst_rtsp_media_get_latency(media) << '\n';
    // gst_rtsp_media_factory_set_shared(factory, TRUE);
    // gst_rtsp_media_set_reusable(media, TRUE);
    // gst_rtsp_media_unprepare(media);

    /* make sure ther datais freed when the media is gone */
    // g_object_set_data_full (G_OBJECT (media), "my-extra-data", ctx,
    // (GDestroyNotify) g_free);

    /* install the callback that will be called when a buffer is needed */
    g_signal_connect (appsrc, "need-data", (GCallback) needData, ctx);
    //g_signal_connect (appsrc, "need-data", G_CALLBACK (start_feed), );
    //g_signal_connect (appsrc, "enough-data", G_CALLBACK (stop_feed), );
    gst_object_unref (appsrc);
    gst_object_unref (element);
}

GstRTSPMediaFactory* RtspServer::createRTSPMediaFactory(Context* ctx)
{
    GstRTSPMediaFactory* factory;
    factory = gst_rtsp_media_factory_new ();

    char *outAppsrc = new char[300];
    sprintf(outAppsrc, "( appsrc name=mysrc is-live=true caps=video/x-raw,format=BGR,width=%d,height=%d,framerate=%d/1 !  videoconvert ! nvvidconv ! nvv4l2h264enc preset-level=UltraFastPreset maxperf-enable=1 ! rtph264pay config-interval=1 name=pay0 pt=96 )",
        int(ctx->outWidth), int(ctx->outHeight), int(ctx->outFPS));
    // sprintf(outAppsrc, "( appsrc name=mysrc is-live=true caps=video/x-raw,format=BGR,width=%d,height=%d,framerate=%d/1 ! videoconvert ! nvvidconv ! omxh264enc preset-level=UltraFastPreset ! rtph264pay config-interval=1 name=pay0 pt=96 )",
    //     int(context.outWidth), int(context.outHeight), int(context.outFPS));
    // sprintf(outAppsrc, "( appsrc name=mysrc is-live=true block=true format=GST_FORMAT_TIME caps=video/x-raw,format=BGR,width=%d,height=%d,framerate=%d/1 ! videoconvert ! video/x-raw,format=I420 ! x264enc speed-preset=ultrafast tune=zerolatency ! rtph264pay config-interval=1 name=pay0 pt=96 )",
    //     int(context.outWidth), int(context.outHeight), int(context.outFPS));
    gst_rtsp_media_factory_set_launch (factory, outAppsrc);
    gst_rtsp_media_factory_set_shared(factory, TRUE);
    // gst_rtsp_media_factory_set_enable_rtcp(factory, TRUE);
    g_signal_connect (factory, "media-configure", (GCallback) mediaConfigure, (void*)ctx);
    return factory;
}

void RtspServer::run(const IOutput::Param& param)
{
    context.timestamp = 0;
    context.outWidth = param.Width;
    context.outHeight = param.Height;
    context.outFPS = param.FPS;
    context.outIndex = param.Index;
    context.outPort = param.Port;
    context.count = 0;
    context.frameImage = cv::Mat::ones(param.Width, param.Height, CV_8UC3);
    context.loop = nullptr;

    // Params prm = param;
    GstFlowReturn ret;
    GstElement *pipeline;

    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;

    gst_init (NULL,NULL);
    context.loop = g_main_loop_new (NULL, FALSE);

    server = gst_rtsp_server_new ();
    g_object_set (server, "service", context.outPort.c_str(), NULL);

    mounts = gst_rtsp_server_get_mount_points (server);
    factory = createRTSPMediaFactory(&context);

    g_signal_connect (server, "client-connected", (GCallback) clientConnected, (void*)&context);
    /* attach the test factory to the /test url */
    gst_rtsp_mount_points_add_factory (mounts, context.outIndex.c_str(), factory);

    /* don't need the ref to the mounts anymore */
    g_object_unref (mounts);

    /* attach the server to the default maincontext */
    gst_rtsp_server_attach (server, NULL);

    g_print ("stream ready at rtsp://127.0.0.1:%s%s\n",context.outPort.c_str(),context.outIndex.c_str());
    std::cout << "start to run the rtsp server...\n";

    /* start serving */
    // isRunning = true;
    g_main_loop_run (context.loop);
    std::cout << "exited the rtsp server...\n";
    // isRunning = false;
}

void RtspServer::clientConnected(GstRTSPServer* server, GstRTSPClient* client, gpointer user_data)
{
    g_print("client connected %p\n", client);
    g_signal_connect(client, "closed", (GCallback)clientClosed,
                    user_data);
}

void RtspServer::clientClosed(GstRTSPClient* client, gpointer user_data)
{
    Context* ctx = (Context*)user_data;

    g_print("client closed %p\n", client);

    GstRTSPSessionPool *pool;
    pool = gst_rtsp_client_get_session_pool(client);
    std::cout << "pool address: " << pool << '\n';

    guint numMaxSession = gst_rtsp_session_pool_get_max_sessions(pool);
    std::cout << "maximum number of sessions: " << numMaxSession << '\n';

    guint numCurrentSession = gst_rtsp_session_pool_get_n_sessions(pool);
    std::cout << "number of current active session: " << numCurrentSession << '\n';

    removeSession(client);
    // disconnect(client);
}

// gboolean RtspServer::disconnect(GstRTSPClient* client)
// {
//     std::cout << "closing rtsp client...\n";
//     gst_rtsp_client_close(client);
//     return TRUE;
// }

gboolean RtspServer::removeSession(GstRTSPClient* client)
{
    std::cout << "removing session pool...\n";
    GstRTSPSessionPool *pool;
    pool = gst_rtsp_client_get_session_pool(client);
    std::cout << "pool address: " << pool << '\n';

    guint removed = gst_rtsp_session_pool_cleanup (pool);
    g_object_unref (pool);
    g_print("Removed %d sessions\n", removed);  
}

// void RtspServer::tearDown(GstRTSPClient* client, GstRTSPContext* ctx, gpointer user_data)
// {
//     std::cout << "tear down...\n";
//     removeSession(client);
//     disconnect(client);
// }

void RtspServer::start(const IOutput::Param& param)
{
    t_server = std::thread([&, obj=this]()
    {
        obj->run(param);
    });
    t_server.detach();
}

bool ScreenWriter::open(const IOutput::Param& params)
{
    if(openFlag)
        return true;
    
    displayWidth = params.Width;
    displayHeight = params.Height;
    windowName = params.OutType;
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
    cv::Mat displayImg;
    cv::resize(image, displayImg, cv::Size{displayWidth, displayHeight},
               0,0,cv::INTER_AREA);
    cv::imshow(windowName,displayImg);

    int keycode = cv::waitKey(10) & 0xff ; 
    if (keycode == 27)
    {
        openFlag = false;
        std::cout << "get key value: " << keycode << "\n";
    }
        
    return true;
}

PlayManager::PlayManager(int bufSize) : bufferSize(bufSize),
                                        imagePool(bufSize),
                                        cap(nullptr)
{
    assert(input!=nullptr);
    playStatus = Status::STOP; 
    for(int i=0; i<bufSize; ++i)
    {
        imagePool.push_back(cv::Mat{});
    }
}

PlayManager::~PlayManager()
{
    stop();
}

bool PlayManager::start(const std::shared_ptr<IInput>& input)
{
    cap = input;
    if(!cap->isOpen())
    {
        std::cout << "the input source is not open...\n";
        return false;
    }
    if(playStatus == Status::STOP)
    {
        playStatus = Status::RUN;
        receiveThread = std::thread([obj=this]()
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

    cap.reset();
}

bool PlayManager::read(cv::Mat& outImage)
{
    std::unique_lock<std::mutex> bufferLock(bufferMutex);
    if(buffer.empty())
    {
        return false;
    }
    cv::Mat img = buffer.front();
    buffer.pop();
    bufferLock.unlock();

    std::swap(img, outImage);

    std::unique_lock<std::mutex> poolLock(poolMutex);
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
        std::unique_lock<std::mutex> poolLock(poolMutex);
        if(!imagePool.empty())
        {
            // fetch from image pool
            img = imagePool.back();
            imagePool.pop_back();
            poolLock.unlock();
            // std::cout << "from capture image pool...\n";
        }
        else 
        {
            // fetch from buffer
            poolLock.unlock();
            std::unique_lock<std::mutex> bufferLock(bufferMutex);
            img = buffer.front();
            buffer.pop();
            std::cout << "drop one frame in capture...\n";
            bufferLock.unlock();
            // std::cout << "from capture buffer...\n";
        }
        
        // read image
        if(!cap->read(img))
        {
            poolLock.lock();
            imagePool.push_back(img);
            poolLock.unlock();

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
            std::unique_lock<std::mutex> bufferLock(bufferMutex);
            buffer.push(img);
            bufferLock.unlock();
        }
    }
}

};