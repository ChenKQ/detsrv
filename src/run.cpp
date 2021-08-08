#include "detectionservice.h"
#include "utils.h"
#include "config.h"

int main(int argc, const char** argv)
{
    detsvr::Logger& logger = detsvr::Logger::CreateInstance();
    logger.initialize("DetectionService", "detectionsvr");

    detsvr::Config::load("./config.txt");
    detsvr::Config& cfg = detsvr::Config::GetInstance();

    if(!detsvr::License::Legal(cfg.licenseCfg.license, cfg.svrCfg.NIC))
    {
         logger.Log("Illegal license...");
         return 0;
    }

    detsvr::DetectionService svr(cfg, logger);
    
    int errorCode = svr.initialize();
    if(errorCode != 0)
    {
        return errorCode;
    }
    
    svr.start();
    return 0;
}