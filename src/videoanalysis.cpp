#include "config.h"
#include "detcore/plugincore.h"
#include "detcore/utils.h"
#include "detcore/detection.h"
#include "detcore/io.h"
#include "detcore/factory.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

int main(int argc, char* argv[])
{
    detsvr::Logger& logger = detsvr::Logger::CreateInstance();
    logger.initialize("VideoAnalysis", "videoanalysis");

    detsvr::Config::load("./config.json");
    detsvr::Config& cfg = detsvr::Config::GetInstance();

    // std::shared_ptr<detsvr::IDetect> pDetector = 
    //         detsvr::PluginCore::CreateDetector(cfg.pluginCfg.filename.c_str());
    detsvr::IInput::Param& inParam = cfg.inParam;
    std::shared_ptr<detsvr::IInput> pReader = 
        detsvr::Factory<detsvr::IInput>::CreateInstance(inParam.InType);
    if(!pReader->open(inParam))
    {
        return -1;
    }

    detsvr::IOutput::Param& outParam = cfg.outParam;
    // detsvr::IOutput::Param outParam 
    // {
    //     OutType : "rtsp",
    //     Protocol : "rtsp",
    //     Ip : "172.20.10.9",
    //     Port : "8554",
    //     Index : "/live/test2",
    //     Width : 1920,
    //     Height : 1080,
    //     // Width : 1280,
    //     // Height : 720,
    //     FPS : 30
    // };
    // detsvr::IOutput::Param outParam 
    // {
    //     OutType : "rtmp",
    //     Protocol : "rtmp",
    //     Ip : "172.20.10.9",
    //     Port : "1935",
    //     Index : "/live/test2",
    //     Width : 1920,
    //     Height : 1080,
    //     // Width : 1280,
    //     // Height : 720,
    //     FPS : 30
    // };
    std::shared_ptr<detsvr::IOutput> pWriter = 
        detsvr::Factory<detsvr::IOutput>::CreateInstance(outParam.OutType);
    if(!pWriter->open(outParam))
    {
        return -1;
    }

    detsvr::PlayManager pm(pReader, 8);
    if(!pm.start())
    {
        return -1;
    }

    detsvr::WriteManager wm(pWriter, 8);
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

        // pReader->read(img);
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

        // pWriter->write(img);
        wm.write(img, result);
    }

    pm.stop();
    // pWriter->close();
    pReader->close();
    return 0;
}