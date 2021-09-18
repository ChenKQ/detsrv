#include "detcore/plugincore.h"
#include "detcore/detection.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <memory>

int main(int argc, char* argv[])
{
    std::cout << "test of plugin ...\n";
    detsvr::PluginFactory factory;
    std::shared_ptr<detsvr::IDetect> pDetector = 
            factory.CreateDetector("./libminic.so");
    if(pDetector == nullptr)
    {
        std::cout << "empty pointer...\n";
    }

    const char* imgFile = argv[1];
    std::ifstream fstrm(imgFile, std::ios::binary);
    std::stringstream sstrm;
    sstrm << fstrm.rdbuf();
    std::string str = sstrm.str();
    detsvr::DetectionResult result = 
            pDetector->detect(str.c_str(), str.length());
    std::cout   << "{img_width: " << result.img_width 
                << ", img_height: " << result.img_height
                << ", pre_time: " << result.pre_time
                << ", inf_time: " << result.inf_time
                << ", list: " << result.list.size() << "}\n";    
    return 0;
}