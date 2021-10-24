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
    int frameNumber = cap.get(cv::CAP_PROP_FRAME_COUNT);
    while(true)
    {
        int currentPos = cap.get(cv::CAP_PROP_POS_FRAMES);
        if(currentPos >= frameNumber-1)
        {
            cap.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }

        if(wm.getStatus()!=detsvr::WriteManager<Result>::RUN)
        {
            // pm.stop();
            std::cerr << "Error: the output status is: " << 
                static_cast<detsvr::WriteManager<Result>::Status>(wm.getStatus());
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