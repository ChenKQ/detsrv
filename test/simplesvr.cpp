#include "../src/service.h"
#include "../src/utils.h"

int main(int argc, const char **argv) 
{
    detsvr::Logger& logger = detsvr::Logger::CreateInstance();
    logger.initialize("SimpleService", "simplesvr");
    
    detsvr::SimpleService svr(logger);
    
    int errorCode = svr.initialize();
    if(errorCode != 0)
    {
        return errorCode;
    }
    
    svr.start();
}