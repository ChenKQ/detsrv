#include "../config.h"
#include "../plugincore.h"
#include "../utils.h"
#include "detsvr/IDetect.h"
#include "detsvr/detsvr.h"
#include "videoreader.h"
#include "playmanager.h"
#include "rtmpwriter.h"
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

    std::shared_ptr<detsvr::IDetect> pDetector = 
            detsvr::PluginCore::CreateDetector(cfg.pluginCfg.filename.c_str());

    std::string rtspuri = "rtsp://192.168.1.7:8554/live/test1";
    // int capture_width = 1280 ;
    // int capture_height = 720 ;
    int display_width = 1920 ;
    int display_height = 1080 ;
    // int display_width = 1280 ;
    // int display_height = 720 ;
    int framerate = 30 ;
    // int flip_method = 2 ;

    // cv::namedWindow("CSI Camera", cv::WINDOW_AUTOSIZE);
    std::cout << "Hit ESC to exit" << "\n" ;
    
    std::shared_ptr<detsvr::IInput> pReader = 
        detsvr::Factory<detsvr::IInput>::CreateInstance("rtsp");
    detsvr::PlayManager pm(pReader, 8);
    if(!pReader->open((void*)rtspuri.c_str()))
    {
        return -1;
    }

    if(!pm.start())
    {
        return -1;
    }

    std::string rtmpWriteUri = "rtmp://192.168.1.7:1935/live/test2";
    detsvr::RtmpWriter writer(4);
    auto pf = [&](cv::Mat& image, const detsvr::DetectionResult& result, 
                           cv::VideoWriter& w)
    {
        for(const detsvr::BBox& box : result.list)
        {
            cv::Point2d tl(box.minx, box.miny);
            cv::Point2d br(box.maxx, box.maxy);
            cv::rectangle(image, tl, br, cv::Scalar(0x27, 0xC1, 0x36), 2);
            cv::putText(image, box.name, cv::Point(box.minx, box.maxy - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
        }

        w.write(image);
        // w << image;
        // cv::imshow("CSI Camera",image);
        // int keycode = cv::waitKey(1) & 0xff ; 
        // if (keycode == 27) 
        // {
        //     writer.close();
        //     cap.close();
        // }
        // std::this_thread::sleep_for(std::chrono::milliseconds(20));
    };

    writer.open(rtmpWriteUri, framerate, display_width, display_height, true, pf);

    cv::Mat img;
    detsvr::DetectionResult result;

    
    int count = 0;
    while(true)
    {
    	if (!pm.read(img)) 
        {
            // std::cout<<"Capture read error"<<std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
	    }
        ++count;
        if(count %1 == 0)
        {
            result = pDetector->detect(img.rows, img.cols, img.type(), img.data, img.step);
            std::cout   << "{img_width: " << result.img_width 
                << ", img_height: " << result.img_height
                << ", pre_time: " << result.pre_time
                << ", inf_time: " << result.inf_time
                << ", list: " << result.list.size() << "}\n";
        }   
        
        // for(const detsvr::BBox& box : result.list)
        // {
        //     cv::Point2d tl(box.minx, box.miny);
        //     cv::Point2d br(box.maxx, box.maxy);
        //     cv::rectangle(img, tl, br, cv::Scalar(0x27, 0xC1, 0x36), 2);
        //     cv::putText(img, box.name, cv::Point(box.minx, box.maxy - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
        // }
        
        writer.write(img, result);
    }

    pm.stop();
    pReader->close();
    // cv::destroyAllWindows();
    return 0;
}