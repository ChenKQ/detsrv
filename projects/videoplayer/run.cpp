#include "config.h"
#include "detcore/utils.h"
#include "detcore/factory.h"
#include "detcore/io.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

typedef struct _NullType
{
} NullType;

int main(int argc, char* argv[])
{
    using Result = NullType;

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

    detsvr::PlayManager pm(8);
    if(!pm.start(pReader))
    {
        return -1;
    }

    auto func = 
        [=](cv::Mat& img, Result& result) 
        {
            return pWriter->write(img);
        };

    detsvr::WriteManager<Result> wm(8);
    if(!wm.start(func))
    {
        return -1;
    }

    cv::Mat img;
    Result result;

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

        if(wm.getStatus()!=detsvr::WriteManager<Result>::RUN)
        {
            pm.stop();
            std::cerr << "Error: the output status is: " << 
                static_cast<detsvr::WriteManager<Result>::Status>(wm.getStatus());
            return -1;
        }

        // pReader->read(img);
    	if (!pm.read(img)) 
        {
            // std::cout<<"Capture read error"<<std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
	    }

        cv::resize(img, img, cv::Size{outParam.Width, outParam.Height});
        
        // pWriter->write(img);
        wm.write(img, result);
        std::cout<<"write frame: "<< ++count << std::endl;
    }

    pm.stop();
    pWriter->close();
    pReader->close();
    return 0;
}