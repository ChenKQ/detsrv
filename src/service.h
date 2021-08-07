#ifndef _DETSVR_SERVICE_
#define _DETSVR_SERVICE_

#include "httplib.h"
#include "utils.h"

#include <string>

namespace detsvr
{
class Service
{
public:
    virtual ~Service() = default;
    
    virtual int initialize() = 0;
    virtual void start() = 0;

protected:
    httplib::Server svr;
};

class SimpleService final: public Service
{
public:
    SimpleService(Logger& m_logger);
    virtual ~SimpleService() override = default;

    int initialize() override;
    void start() override;

private:
    Logger& m_logger;
};

} // end namespace detsvr

#endif