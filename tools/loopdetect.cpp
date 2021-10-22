#include "../src/config.h"
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
    logger.initialize("loopdetect", "loopdetect");

    detsvr::Config::load("./config-loopdetect.json");
    detsvr::Config& cfg = detsvr::Config::GetInstance();

    detsvr::PluginFactory factory;
    std::shared_ptr<detsvr::IDetect> pDetector = 
            factory.CreateDetector(cfg.pluginCfg.filename.c_str());
    detsvr::IInput::Param& inParam = cfg.inParam;
    cv::VideoCapture cap;
    std::cout << "open the file: " << inParam.Uri << '\n';
    cap.open(inParam.Uri);

    detsvr::IOutput::Param& outParam = cfg.outParam;
    std::shared_ptr<detsvr::IOutput> pWriter = 
        detsvr::Factory<detsvr::IOutput>::CreateInstance(outParam.OutType);
    if(!pWriter->open(outParam))
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
    int frameNumber = cap.get(cv::CAP_PROP_FRAME_COUNT);
    while(true)
    {
        int currentPos = cap.get(cv::CAP_PROP_POS_FRAMES);
        if(currentPos >= frameNumber-1)
        {
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }

        if(wm.getStatus()!=detsvr::WriteManager::RUN)
        {
            // pm.stop();
            std::cerr << "Error: the output status is: " << 
                static_cast<detsvr::WriteManager::Status>(wm.getStatus());
            return -1;
        }

        bool ret = cap.read(img);
        if((!ret) || img.empty())
        {
            std::cout << "failed to load or empty image\n";
            continue;
        }
        result = pDetector->detect(img.rows, img.cols, img.type(), img.data, img.step);
        ++count;
        if(count %1000 == 0)
        {
            std::cout   << "{img_width: " << result.img_width 
                << ", img_height: " << result.img_height
                << ", pre_time: " << result.pre_time
                << ", inf_time: " << result.inf_time
                << ", list: " << result.list.size() << "}\n";
        }   

        // pWriter->write(img);
        wm.write(img, result);
    }

    // pm.stop();
    pWriter->close();
    // pReader->close();
    return 0;
}