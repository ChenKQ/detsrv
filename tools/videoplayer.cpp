#include "../src/config.h"
#include "detcore/plugincore.h"
#include "detcore/utils.h"
#include "detcore/detection.h"
#include "detcore/factory.h"
#include "detcore/io.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

int main(int argc, char* argv[])
{
    detsvr::Logger& logger = detsvr::Logger::CreateInstance();
    logger.initialize("videoplayer", "videoplayer");

    detsvr::Config::load("./config-videoplayer.json");
    detsvr::Config& cfg = detsvr::Config::GetInstance();

    detsvr::IInput::Param inParam = cfg.inParam;
    std::shared_ptr<detsvr::IInput> pReader = 
        detsvr::Factory<detsvr::IInput>::CreateInstance(inParam.InType);
    if(!pReader->open(inParam))
    {
        return -1;
    }

    detsvr::IOutput::Param& outParam = cfg.outParam;
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
        
        // pWriter->write(img);
        wm.write(img, result);
        std::cout<<"write frame: "<< ++count << std::endl;
    }

    pm.stop();
    // pWriter->close();
    pReader->close();
    return 0;
}