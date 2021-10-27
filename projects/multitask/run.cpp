#include "config.h"
#include "detcore/plugincore.h"
#include "detcore/utils.h"
#include "detection.h"
#include "detcore/io.h"
#include "detcore/factory.h"
// #include "inference.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

typedef struct PostProcess
{
    using Result = detsvr::DetectionResult;

    static inline bool Act(cv::Mat& img, Result& result)
    {
        static const std::vector<cv::Vec3b> segColor{cv::Vec3b(0, 0, 0), cv::Vec3b(0, 255, 0), cv::Vec3b(255, 0, 0)};
        // static const std::vector<cv::Vec3b> laneColor{cv::Vec3b(0, 0, 0), cv::Vec3b(0, 0, 255), cv::Vec3b(0, 0, 0)};
        // cv::Mat cvt_img_cpu = img;
        // cvt_img.download(cvt_img_cpu);

        // handling seg and lane results
        for (int row = 0; row < img.rows; ++row) 
        {
            uchar* pdata = img.data + row * img.step;
            for (int col = 0; col < img.cols; ++col) 
            {
                int seg_idx = result.segResult.at<int>(row, col);
                // int lane_idx = result.segLane.at<int>(row, col);
                //std::cout << "enter" << ix << std::endl;
                for (int i = 0; i < 3; ++i) {
                    // if (lane_idx) {
                    //     if (i != 2)
                    //         pdata[i] = pdata[i] / 2 + laneColor[lane_idx][i] / 2;
                    // }
                    if (seg_idx)
                        pdata[i] = pdata[i] / 2 + segColor[seg_idx][i] / 2;
                }
                pdata += 3;
            }
        }

        for(const auto& box : result.list)
        {
            cv::Point2d tl(box.minx, box.miny);
            cv::Point2d br(box.maxx, box.maxy);
            cv::rectangle(img, tl, br, cv::Scalar(0x27, 0xC1, 0x36), 2);
            cv::putText(img, box.name, cv::Point(box.minx, box.maxy - 1), cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0xFF, 0xFF, 0xFF), 2);
        }
    }
} PostProcess;


// int main(int argc, char* argv[])
// {
//     using Result = detsvr::DetectionResult;

//     detsvr::Config::load("./config-multitask.json");
//     detsvr::Config& cfg = detsvr::Config::GetInstance();

//     detsvr::PluginFactory<detsvr::IDetect> factory;
//     std::shared_ptr<detsvr::IDetect> pDetector =
//             factory.CreateInstance(cfg.pluginCfg.filename.c_str());
//     detsvr::IInput::Param& inParam = cfg.inParam;
//     std::shared_ptr<detsvr::IInput> pReader = 
//         detsvr::Factory<detsvr::IInput>::CreateInstance(inParam.InType);
//     if(!pReader->open(inParam))
//     {
//         return -1;
//     }

//     detsvr::IOutput::Param& outParam = cfg.outParam;
//     std::shared_ptr<detsvr::IOutput> pWriter = 
//         detsvr::Factory<detsvr::IOutput>::CreateInstance(outParam.OutType);
//     if(!pWriter->open(outParam))
//     {
//         return -1;
//     }

//     cv::Mat img;
//     Result result;

//     int count = 0;
//     while(true)
//     {
//         if(!pReader->isOpen() || !pWriter->isOpen())
//         {
//             return -1;
//         }

//     	if (!pReader->read(img)) 
//         {
//             // std::cout<<"Capture read error"<<std::endl;
//             std::this_thread::sleep_for(std::chrono::milliseconds(10));
//             continue;
// 	    }
        
//         ++count;
//         if(count %1 == 0)
//         {
//             result = pDetector->detect(img.rows, img.cols, img.type(), img.data, img.step);
//             std::cout   << "{img_width: " << result.img_width 
//                 << ", img_height: " << result.img_height
//                 << ", pre_time: " << result.pre_time
//                 << ", inf_time: " << result.inf_time
//                 << ", list: " << result.list.size() << "}\n";
//         }
//         PostProcess::Act(img, result);   

//         pWriter->write(img);
//         // wm.write(img, result);
//     }

//     pWriter->close();
//     pReader->close();
//     return 0;
// }

int main(int argc, char* argv[])
{
    using Result = detsvr::DetectionResult;

    detsvr::Logger& logger = detsvr::Logger::CreateInstance();
    logger.initialize("MultiTask", "multitask");

    detsvr::Config::load("./config-multitask.json");
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

    // detsvr::PlayManager pm(8);
    // if(!pm.start(pReader))
    // {
    //     return -1;
    // }

    auto writeAction = 
        [=](cv::Mat& img, Result& result)
        {
            if(!pWriter->isOpen())
            {
                std::cout << "Exit the writing thread since the writer is not open...\n";
                // status = Status::STOP;
                return false;
            }
            PostProcess::Act(img, result);
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

        if(!cap.isOpened() || !pWriter->isOpen())
        {
            // pm.stop();
            wm.stop();
            return -1;
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
            // std::cout<<"Capture read error"<<std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
	    }
        
        ++count;
        if(count %1 == 0)
        {
            result = pDetector->detect(img.rows, img.cols, img.type(), img.data, img.step);
            // std::this_thread::sleep_for(std::chrono::milliseconds(100));
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