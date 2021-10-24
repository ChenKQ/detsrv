#include "config.h"
#include "detcore/plugincore.h"
#include "detcore/utils.h"
#include "detection.h"
#include "detcore/io.h"
#include "detcore/factory.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

int main(int argc, char* argv[])
{
    using Result = detsvr::DetectionResult;

    detsvr::Logger& logger = detsvr::Logger::CreateInstance();
    logger.initialize("UAVVideo", "uavvideo");

    detsvr::Config::load("./config-uavvideo.json");
    detsvr::Config& cfg = detsvr::Config::GetInstance();

    detsvr::PluginFactory<detsvr::IDetect> factory;
    std::shared_ptr<detsvr::IDetect> pDetector = 
            factory.CreateInstance(cfg.pluginCfg.filename.c_str());
    detsvr::IInput::Param& inParam = cfg.inParam;
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

    auto writeAction = 
        [=](cv::Mat& img, Result& result)
        {
            if(!pWriter->isOpen())
            {
                std::cout << "Exit the writing thread since the writer is not open...\n";
                // status = Status::STOP;
                return false;
            }

            for(const auto& box : result.list)
            {
                cv::Point2d tl(box.minx, box.miny);
                cv::Point2d br(box.maxx, box.maxy);
                cv::rectangle(img, tl, br, cv::Scalar(0x27, 0xC1, 0x36), 2);
                cv::putText(img, box.name, cv::Point(box.minx, box.maxy - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
            }

            return pWriter->write(img);
        };

    detsvr::WriteManager<Result> wm(8);
    if(!wm.start(writeAction))
    {
        return -1;
    }

    cv::Mat img;
    Result result;

    int count = 0;
    while(true)
    {
        if(!pReader->isOpen() || !pWriter->isOpen())
        {
            pm.stop();
            wm.stop();
            return -1;
        }

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

        // pWriter->write(img);
        wm.write(img, result);
    }

    pm.stop();
    pWriter->close();
    pReader->close();
    return 0;
}