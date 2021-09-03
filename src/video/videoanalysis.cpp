#include "../config.h"
#include "../plugincore.h"
#include "../utils.h"
#include "detsvr/IDetect.h"
#include "detsvr/detsvr.h"
#include "io.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

std::string gstreamer_camera_pipeline (int capture_width, int capture_height, 
                                int display_width, int display_height, 
                                int framerate, int flip_method) 
{
    return "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)" + 
            std::to_string(capture_width) + ", height=(int)" +
            std::to_string(capture_height) + ", format=(string)NV12, framerate=(fraction)" + 
            std::to_string(framerate) +
           "/1 ! nvvidconv flip-method=" + 
           std::to_string(flip_method) + " ! video/x-raw, width=(int)" + 
           std::to_string(display_width) + ", height=(int)" +
           std::to_string(display_height) + ", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
}


int main(int argc, char* argv[])
{
    detsvr::Logger& logger = detsvr::Logger::CreateInstance();
    logger.initialize("VideoAnalysis", "videoanalysis");

    detsvr::Config::load("./config.txt");
    detsvr::Config& cfg = detsvr::Config::GetInstance();

    // std::shared_ptr<detsvr::IDetect> pDetector = 
    //         detsvr::PluginCore::CreateDetector(cfg.pluginCfg.filename.c_str());

    // cv::namedWindow("CSI Camera", cv::WINDOW_AUTOSIZE);
    // std::cout << "Hit ESC to exit" << "\n" ;
    
    std::shared_ptr<detsvr::IInput> pReader = 
        detsvr::Factory<detsvr::IInput>::CreateInstance("csi");
    detsvr::PlayManager pm(pReader, 8);
    std::string srcUri = "rtsp://172.20.10.9:8554/live/test1";
    if(!pReader->open(&srcUri))
    {
        return -1;
    }
    if(!pm.start())
    {
        return -1;
    }

    std::shared_ptr<detsvr::IOutput> pWriter = 
        detsvr::Factory<detsvr::IOutput>::CreateInstance("rtmp");
    detsvr::WriteManager wm(pWriter, 8);
    detsvr::RtmpWriter::Params params 
    {
        uri: "rtmp://172.20.10.9:1935/live/test2",
        fps: 30,
        displayWidth: 1920,
        displayHeight: 1080,
        // displayWidth: 1280,
        // displayHeight: 720,
        isColor: true
    };
    // detsvr::ScreenWriter::Params params {"Display"};
    if(!pWriter->open((void*)&params))
    {
        return -1;
    }
    if(!wm.start())
    {
        return -1;
    }

    cv::Mat img;
    detsvr::DetectionResult result;

    int count = 0;
    while(true)
    {
        if(pm.getStatus()!=detsvr::PlayManager::RUN)
        {
            wm.stop();
            std::cerr << "Error: the input is status is: " << 
                static_cast<detsvr::PlayManager::Status>(pm.getStatus());
            return -1;
        }

        if(wm.getStatus()!=detsvr::WriteManager::RUN)
        {
            pm.stop();
            std::cerr << "Error: the output status is: " << 
                static_cast<detsvr::WriteManager::Status>(wm.getStatus());
            return -1;
        }

    	if (!pm.read(img)) 
        {
            // std::cout<<"Capture read error"<<std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
	    }
        ++count;
        // if(count %1 == 0)
        // {
        //     result = pDetector->detect(img.rows, img.cols, img.type(), img.data, img.step);
        //     std::cout   << "{img_width: " << result.img_width 
        //         << ", img_height: " << result.img_height
        //         << ", pre_time: " << result.pre_time
        //         << ", inf_time: " << result.inf_time
        //         << ", list: " << result.list.size() << "}\n";
        // }   
        
        wm.write(img, result);
    }

    pm.stop();
    pReader->close();
    // cv::destroyAllWindows();
    return 0;
}