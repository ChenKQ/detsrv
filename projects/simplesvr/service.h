#ifndef DETSVR_SERVICE_H
#define DETSVR_SERVICE_H

#include "httplib.h"
#include "detcore/utils.h"
#include <string>

namespace detsvr
{

class SimpleService final
{
public:
    SimpleService(Logger& m_logger);
    ~SimpleService() = default;

    int initialize() ;
    void start() ;

private:
    httplib::Server svr;
    Logger& m_logger;
};

} // end namespace detsvr

#endif